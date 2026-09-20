#include "backend/ai/AIProvider.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <json/json.h>

#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace edu_ai::ai {
namespace {

constexpr std::size_t kMaxHttpResponseBytes = 64U * 1024U * 1024U;
constexpr long kSocketTimeoutSeconds = 300L;
constexpr int kSocketTimeoutMilliseconds = 300'000;

struct HttpEndpoint {
  std::string host;
  std::string port;
  std::string host_header;
  std::string chat_path;
};

struct HttpResponseMetadata {
  int status_code{};
  bool chunked{};
  std::optional<std::size_t> content_length;
};

class SocketHandle final {
 public:
  explicit SocketHandle(int descriptor) : descriptor_(descriptor) {}

  ~SocketHandle() {
    if (descriptor_ >= 0)
      ::close(descriptor_);
  }

  SocketHandle(const SocketHandle&) = delete;
  SocketHandle& operator=(const SocketHandle&) = delete;

  int get() const {
    return descriptor_;
  }

 private:
  int descriptor_;
};

bool is_ascii_space_or_control(char value) {
  const auto character = static_cast<unsigned char>(value);
  return character <= 0x20U || character == 0x7fU;
}

std::string trim_ascii(std::string_view value) {
  std::size_t begin{};
  while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])))
    ++begin;

  std::size_t end = value.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1U])))
    --end;

  return std::string(value.substr(begin, end - begin));
}

std::string lowercase_ascii(std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (const char character : value)
    result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
  return result;
}

std::string normalize_port(std::string_view port) {
  if (port.empty())
    throw std::invalid_argument("Ollama base URL has an empty port.");

  std::uint64_t parsed{};
  for (const char character : port) {
    if (character < '0' || character > '9')
      throw std::invalid_argument("Ollama base URL has an invalid port.");
    const auto digit = static_cast<std::uint64_t>(character - '0');
    if (parsed > (65535U - digit) / 10U)
      throw std::invalid_argument("Ollama base URL port must be between 1 and 65535.");
    parsed = parsed * 10U + digit;
  }
  if (parsed == 0U)
    throw std::invalid_argument("Ollama base URL port must be between 1 and 65535.");
  return std::to_string(parsed);
}

HttpEndpoint parse_endpoint(const std::string& base_url) {
  constexpr std::string_view kHttpPrefix{"http://"};
  if (!base_url.starts_with(kHttpPrefix))
    throw std::invalid_argument("OllamaProvider supports only http:// base URLs.");
  if (base_url.empty() ||
      std::any_of(base_url.begin(), base_url.end(), [](char character) { return is_ascii_space_or_control(character); }))
    throw std::invalid_argument("Ollama base URL must not contain whitespace or control characters.");

  const std::string_view remainder{base_url.data() + kHttpPrefix.size(), base_url.size() - kHttpPrefix.size()};
  const auto authority_end = remainder.find_first_of("/?#");
  const std::string_view authority = remainder.substr(0, authority_end);
  if (authority.empty())
    throw std::invalid_argument("Ollama base URL must include a host.");
  if (authority.find('@') != std::string_view::npos)
    throw std::invalid_argument("Ollama base URL must not contain user credentials.");

  std::string_view path;
  if (authority_end != std::string_view::npos) {
    if (remainder[authority_end] != '/')
      throw std::invalid_argument("Ollama base URL must not include a query or fragment.");
    path = remainder.substr(authority_end);
    if (path.find_first_of("?#") != std::string_view::npos)
      throw std::invalid_argument("Ollama base URL must not include a query or fragment.");
  }

  HttpEndpoint endpoint;
  bool ipv6_literal{};
  if (authority.front() == '[') {
    const auto closing_bracket = authority.find(']');
    if (closing_bracket == std::string_view::npos || closing_bracket == 1U)
      throw std::invalid_argument("Ollama base URL has an invalid IPv6 host.");
    endpoint.host = std::string(authority.substr(1U, closing_bracket - 1U));
    ipv6_literal = true;
    const std::string_view port_suffix = authority.substr(closing_bracket + 1U);
    if (!port_suffix.empty()) {
      if (port_suffix.front() != ':')
        throw std::invalid_argument("Ollama base URL has invalid text after its IPv6 host.");
      endpoint.port = normalize_port(port_suffix.substr(1U));
    }
  } else {
    const auto last_colon = authority.rfind(':');
    if (last_colon != std::string_view::npos) {
      if (authority.find(':') != last_colon)
        throw std::invalid_argument("Ollama IPv6 hosts must be enclosed in brackets.");
      endpoint.host = std::string(authority.substr(0, last_colon));
      endpoint.port = normalize_port(authority.substr(last_colon + 1U));
    } else {
      endpoint.host = std::string(authority);
    }
  }

  if (endpoint.host.empty())
    throw std::invalid_argument("Ollama base URL must include a host.");
  if (endpoint.port.empty())
    endpoint.port = "80";

  endpoint.host_header = ipv6_literal ? "[" + endpoint.host + "]" : endpoint.host;
  if (endpoint.port != "80")
    endpoint.host_header += ":" + endpoint.port;

  std::string prefix = std::string(path);
  while (!prefix.empty() && prefix.back() == '/')
    prefix.pop_back();
  endpoint.chat_path = (prefix.empty() ? std::string{} : std::move(prefix)) + "/api/chat";
  return endpoint;
}

void configure_socket_timeouts(int descriptor) {
  timeval timeout{};
  timeout.tv_sec = kSocketTimeoutSeconds;
  if (::setsockopt(descriptor, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0)
    throw std::system_error(errno, std::generic_category(), "Failed to set Ollama socket send timeout");
  if (::setsockopt(descriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
    throw std::system_error(errno, std::generic_category(), "Failed to set Ollama socket receive timeout");
}

void set_socket_nonblocking(int descriptor, bool enabled) {
  const int current_flags = ::fcntl(descriptor, F_GETFL);
  if (current_flags == -1)
    throw std::system_error(errno, std::generic_category(), "Failed to inspect Ollama socket flags");

  const int desired_flags = enabled ? current_flags | O_NONBLOCK : current_flags & ~O_NONBLOCK;
  if (::fcntl(descriptor, F_SETFL, desired_flags) == -1)
    throw std::system_error(errno, std::generic_category(), "Failed to update Ollama socket flags");
}

void connect_with_timeout(int descriptor, const sockaddr* address, socklen_t address_length) {
  if (::connect(descriptor, address, address_length) == 0)
    return;
  if (errno != EINPROGRESS && errno != EWOULDBLOCK)
    throw std::system_error(errno, std::generic_category(), "connect");

  pollfd readiness{.fd = descriptor, .events = POLLOUT, .revents = 0};
  while (true) {
    const int result = ::poll(&readiness, 1, kSocketTimeoutMilliseconds);
    if (result > 0)
      break;
    if (result == 0)
      throw std::system_error(ETIMEDOUT, std::generic_category(), "Timed out connecting to Ollama endpoint");
    if (errno != EINTR)
      throw std::system_error(errno, std::generic_category(), "poll while connecting to Ollama endpoint");
  }

  int connection_error{};
  socklen_t error_length = sizeof(connection_error);
  if (::getsockopt(descriptor, SOL_SOCKET, SO_ERROR, &connection_error, &error_length) != 0)
    throw std::system_error(errno, std::generic_category(), "Failed to inspect Ollama connection status");
  if (connection_error != 0)
    throw std::system_error(connection_error, std::generic_category(), "connect");
}

SocketHandle connect_to(const HttpEndpoint& endpoint) {
  addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  addrinfo* resolved{};
  const int resolution_status = ::getaddrinfo(endpoint.host.c_str(), endpoint.port.c_str(), &hints, &resolved);
  if (resolution_status != 0)
    throw std::runtime_error("Cannot resolve Ollama endpoint " + endpoint.host + ":" + endpoint.port + ": " +
                             ::gai_strerror(resolution_status));
  const std::unique_ptr<addrinfo, decltype(&::freeaddrinfo)> addresses(resolved, &::freeaddrinfo);

  std::string last_error;
  for (auto* address = resolved; address != nullptr; address = address->ai_next) {
    const int descriptor = ::socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (descriptor < 0) {
      last_error = std::system_error(errno, std::generic_category(), "socket").what();
      continue;
    }

    try {
      configure_socket_timeouts(descriptor);
      set_socket_nonblocking(descriptor, true);
      connect_with_timeout(descriptor, address->ai_addr, address->ai_addrlen);
      set_socket_nonblocking(descriptor, false);
      return SocketHandle(descriptor);
    } catch (const std::system_error& error) {
      last_error = error.what();
      ::close(descriptor);
      continue;
    }
  }

  throw std::runtime_error("Cannot connect to Ollama endpoint " + endpoint.host + ":" + endpoint.port +
                           (last_error.empty() ? "." : ": " + last_error));
}

void send_all(int descriptor, std::string_view data) {
  std::size_t sent{};
  while (sent < data.size()) {
#ifdef MSG_NOSIGNAL
    constexpr int kSendFlags = MSG_NOSIGNAL;
#else
    constexpr int kSendFlags = 0;
#endif
    const auto result = ::send(descriptor, data.data() + sent, data.size() - sent, kSendFlags);
    if (result > 0) {
      sent += static_cast<std::size_t>(result);
      continue;
    }
    if (result < 0 && errno == EINTR)
      continue;
    if (result == 0)
      throw std::runtime_error("Ollama connection closed while sending the HTTP request.");
    throw std::system_error(errno, std::generic_category(), "Failed to send Ollama HTTP request");
  }
}

std::optional<std::size_t> header_end_offset(const std::string& response) {
  const auto crlf_end = response.find("\r\n\r\n");
  if (crlf_end != std::string::npos)
    return crlf_end + 4U;
  const auto lf_end = response.find("\n\n");
  if (lf_end != std::string::npos)
    return lf_end + 2U;
  return std::nullopt;
}

std::optional<std::size_t> parse_content_length(std::string_view value) {
  const std::string normalized = trim_ascii(value);
  if (normalized.empty())
    throw std::runtime_error("Ollama HTTP response has an empty Content-Length header.");

  std::size_t length{};
  const auto [position, error] = std::from_chars(normalized.data(), normalized.data() + normalized.size(), length);
  if (error != std::errc{} || position != normalized.data() + normalized.size())
    throw std::runtime_error("Ollama HTTP response has an invalid Content-Length header.");
  return length;
}

HttpResponseMetadata parse_http_headers(std::string_view headers) {
  const auto first_newline = headers.find('\n');
  if (first_newline == std::string_view::npos)
    throw std::runtime_error("Ollama HTTP response is missing a status line terminator.");
  std::string_view status_line = headers.substr(0, first_newline);
  if (!status_line.empty() && status_line.back() == '\r')
    status_line.remove_suffix(1U);
  if (!status_line.starts_with("HTTP/"))
    throw std::runtime_error("Ollama endpoint returned an invalid HTTP status line.");

  const auto status_begin = status_line.find(' ');
  if (status_begin == std::string_view::npos)
    throw std::runtime_error("Ollama endpoint returned an incomplete HTTP status line.");
  const auto status_end = status_line.find(' ', status_begin + 1U);
  const std::string_view status_text =
      status_line.substr(status_begin + 1U, (status_end == std::string_view::npos ? status_line.size() : status_end) -
                                             status_begin - 1U);

  HttpResponseMetadata metadata;
  const auto [status_position, status_error] =
      std::from_chars(status_text.data(), status_text.data() + status_text.size(), metadata.status_code);
  if (status_error != std::errc{} || status_position != status_text.data() + status_text.size())
    throw std::runtime_error("Ollama endpoint returned an invalid HTTP status code.");

  std::size_t line_begin = first_newline + 1U;
  while (line_begin < headers.size()) {
    const auto line_end = headers.find('\n', line_begin);
    const std::size_t line_limit = line_end == std::string_view::npos ? headers.size() : line_end;
    std::string_view line = headers.substr(line_begin, line_limit - line_begin);
    if (!line.empty() && line.back() == '\r')
      line.remove_suffix(1U);
    if (!line.empty()) {
      const auto separator = line.find(':');
      if (separator == std::string_view::npos)
        throw std::runtime_error("Ollama HTTP response contains a malformed header.");
      const std::string name = lowercase_ascii(trim_ascii(line.substr(0, separator)));
      const std::string value = lowercase_ascii(trim_ascii(line.substr(separator + 1U)));
      if (name == "content-length") {
        const auto length = parse_content_length(value);
        if (metadata.content_length && *metadata.content_length != *length)
          throw std::runtime_error("Ollama HTTP response has conflicting Content-Length headers.");
        metadata.content_length = length;
      } else if (name == "transfer-encoding" && value.find("chunked") != std::string::npos) {
        metadata.chunked = true;
      }
    }
    if (line_end == std::string_view::npos)
      break;
    line_begin = line_end + 1U;
  }
  if (metadata.chunked)
    metadata.content_length.reset();
  return metadata;
}

int hex_value(char character) {
  if (character >= '0' && character <= '9')
    return character - '0';
  if (character >= 'a' && character <= 'f')
    return character - 'a' + 10;
  if (character >= 'A' && character <= 'F')
    return character - 'A' + 10;
  return -1;
}

enum class ChunkedState { Incomplete, Complete };

bool has_complete_trailer_section(std::string_view body, std::size_t position) {
  while (true) {
    const auto crlf_end = body.find("\r\n", position);
    const auto lf_end = body.find('\n', position);
    const bool uses_crlf = crlf_end != std::string_view::npos && (lf_end == std::string_view::npos || crlf_end <= lf_end);
    const auto line_end = uses_crlf ? crlf_end : lf_end;
    if (line_end == std::string_view::npos)
      return false;

    std::string_view line = body.substr(position, line_end - position);
    position = line_end + (uses_crlf ? 2U : 1U);
    if (line.empty())
      return true;
    if (line.find(':') == std::string_view::npos)
      throw std::runtime_error("Ollama HTTP response contains a malformed chunk trailer.");
  }
}

ChunkedState inspect_chunked_body(std::string_view body) {
  std::size_t position{};
  while (true) {
    const auto crlf_end = body.find("\r\n", position);
    const auto lf_end = body.find('\n', position);
    const bool uses_crlf = crlf_end != std::string_view::npos && (lf_end == std::string_view::npos || crlf_end <= lf_end);
    const auto line_end = uses_crlf ? crlf_end : lf_end;
    if (line_end == std::string_view::npos)
      return ChunkedState::Incomplete;
    std::string_view size_text = body.substr(position, line_end - position);
    const auto extension = size_text.find(';');
    if (extension != std::string_view::npos)
      size_text = size_text.substr(0, extension);
    const std::string normalized_size = trim_ascii(size_text);
    if (normalized_size.empty())
      throw std::runtime_error("Ollama HTTP response has an empty chunk size.");

    std::size_t chunk_size{};
    for (const char character : normalized_size) {
      const int digit = hex_value(character);
      if (digit < 0 || chunk_size > (std::numeric_limits<std::size_t>::max() - static_cast<std::size_t>(digit)) / 16U)
        throw std::runtime_error("Ollama HTTP response has an invalid chunk size.");
      chunk_size = chunk_size * 16U + static_cast<std::size_t>(digit);
    }
    position = line_end + (uses_crlf ? 2U : 1U);
    if (chunk_size == 0U)
      return has_complete_trailer_section(body, position) ? ChunkedState::Complete : ChunkedState::Incomplete;
    if (chunk_size > body.size() - position)
      return ChunkedState::Incomplete;
    position += chunk_size;
    if (position == body.size())
      return ChunkedState::Incomplete;
    if (body[position] == '\r') {
      if (position + 1U >= body.size())
        return ChunkedState::Incomplete;
      if (body[position + 1U] != '\n')
        throw std::runtime_error("Ollama HTTP response has an invalid chunk terminator.");
      position += 2U;
    } else if (body[position] == '\n') {
      ++position;
    } else {
      throw std::runtime_error("Ollama HTTP response has an invalid chunk terminator.");
    }
  }
}

std::string decode_chunked_body(std::string_view body) {
  std::string decoded;
  std::size_t position{};
  while (true) {
    const auto crlf_end = body.find("\r\n", position);
    const auto lf_end = body.find('\n', position);
    const bool uses_crlf = crlf_end != std::string_view::npos && (lf_end == std::string_view::npos || crlf_end <= lf_end);
    const auto line_end = uses_crlf ? crlf_end : lf_end;
    if (line_end == std::string_view::npos)
      throw std::runtime_error("Ollama HTTP response ended before a chunk size was complete.");
    std::string_view size_text = body.substr(position, line_end - position);
    const auto extension = size_text.find(';');
    if (extension != std::string_view::npos)
      size_text = size_text.substr(0, extension);
    const std::string normalized_size = trim_ascii(size_text);
    if (normalized_size.empty())
      throw std::runtime_error("Ollama HTTP response has an empty chunk size.");

    std::size_t chunk_size{};
    for (const char character : normalized_size) {
      const int digit = hex_value(character);
      if (digit < 0 || chunk_size > (std::numeric_limits<std::size_t>::max() - static_cast<std::size_t>(digit)) / 16U)
        throw std::runtime_error("Ollama HTTP response has an invalid chunk size.");
      chunk_size = chunk_size * 16U + static_cast<std::size_t>(digit);
    }
    position = line_end + (uses_crlf ? 2U : 1U);
    if (chunk_size == 0U) {
      if (!has_complete_trailer_section(body, position))
        throw std::runtime_error("Ollama HTTP response ended before its chunk trailer section was complete.");
      return decoded;
    }
    if (chunk_size > body.size() - position)
      throw std::runtime_error("Ollama HTTP response ended in the middle of a chunk.");
    if (decoded.size() > kMaxHttpResponseBytes - chunk_size)
      throw std::runtime_error("Ollama HTTP response exceeds the 64 MiB safety limit.");
    decoded.append(body.data() + position, chunk_size);
    position += chunk_size;
    if (position + 1U >= body.size())
      throw std::runtime_error("Ollama HTTP response ended before a chunk terminator.");
    if (body[position] == '\r' && body[position + 1U] == '\n') {
      position += 2U;
    } else if (body[position] == '\n') {
      ++position;
    } else {
      throw std::runtime_error("Ollama HTTP response has an invalid chunk terminator.");
    }
  }
}

std::string receive_http_response(int descriptor) {
  std::string received;
  received.reserve(8192U);
  std::optional<std::size_t> body_offset;
  std::optional<HttpResponseMetadata> metadata;

  const auto response_is_complete = [&]() {
    if (!body_offset)
      return false;
    const std::string_view body{received.data() + *body_offset, received.size() - *body_offset};
    if (metadata->content_length) {
      if (*metadata->content_length > kMaxHttpResponseBytes)
        throw std::runtime_error("Ollama HTTP response exceeds the 64 MiB safety limit.");
      return body.size() >= *metadata->content_length;
    }
    return metadata->chunked && inspect_chunked_body(body) == ChunkedState::Complete;
  };

  std::array<char, 8192> buffer{};
  while (true) {
    const auto count = ::recv(descriptor, buffer.data(), buffer.size(), 0);
    if (count > 0) {
      if (received.size() > kMaxHttpResponseBytes - static_cast<std::size_t>(count))
        throw std::runtime_error("Ollama HTTP response exceeds the 64 MiB safety limit.");
      received.append(buffer.data(), static_cast<std::size_t>(count));

      if (!body_offset) {
        body_offset = header_end_offset(received);
        if (body_offset)
          metadata = parse_http_headers(std::string_view(received.data(), *body_offset));
      }
      if (response_is_complete())
        return received;
      continue;
    }
    if (count == 0)
      break;
    if (errno == EINTR)
      continue;
    throw std::system_error(errno, std::generic_category(), "Failed to receive Ollama HTTP response");
  }

  if (!body_offset || !metadata)
    throw std::runtime_error("Ollama connection closed before a complete HTTP response header was received.");
  if (metadata->content_length) {
    const std::size_t actual_length = received.size() - *body_offset;
    if (actual_length < *metadata->content_length)
      throw std::runtime_error("Ollama connection closed before the complete HTTP response body was received.");
  }
  if (metadata->chunked && inspect_chunked_body(
                               std::string_view(received.data() + *body_offset, received.size() - *body_offset)) !=
                              ChunkedState::Complete)
    throw std::runtime_error("Ollama connection closed before the complete chunked response body was received.");
  return received;
}

std::string response_body(const std::string& response, const HttpResponseMetadata& metadata,
                          std::size_t body_offset) {
  const std::string_view body{response.data() + body_offset, response.size() - body_offset};
  if (metadata.chunked)
    return decode_chunked_body(body);
  if (metadata.content_length) {
    if (body.size() < *metadata.content_length)
      throw std::runtime_error("Ollama HTTP response body is shorter than Content-Length.");
    return std::string(body.substr(0, *metadata.content_length));
  }
  return std::string(body);
}

Json::Value parse_json(std::string_view json, const std::string& description) {
  Json::CharReaderBuilder builder;
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value value;
  std::string errors;
  if (!reader->parse(json.data(), json.data() + json.size(), &value, &errors))
    throw std::runtime_error("Cannot parse " + description + " JSON: " + trim_ascii(errors));
  return value;
}

std::string compact_json(const Json::Value& value) {
  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";
  return Json::writeString(writer, value);
}

std::string response_error_detail(std::string_view body) {
  try {
    const Json::Value payload = parse_json(body, "Ollama error response");
    if (payload.isObject() && payload["error"].isString())
      return payload["error"].asString();
  } catch (const std::exception&) {
    // Fall back to a bounded plain-text body below.
  }
  std::string detail = trim_ascii(body);
  constexpr std::size_t kMaxErrorDetail = 512U;
  if (detail.size() > kMaxErrorDetail)
    detail.resize(kMaxErrorDetail);
  return detail;
}

int response_token_count(const Json::Value& root, const char* field) {
  const Json::Value& value = root[field];
  if (value.isNull())
    return 0;
  if (value.isInt()) {
    if (value.asInt() < 0)
      throw std::runtime_error(std::string("Ollama response field ") + field + " must not be negative.");
    return value.asInt();
  }
  if (value.isUInt()) {
    if (value.asUInt() > static_cast<Json::UInt>(std::numeric_limits<int>::max()))
      throw std::runtime_error(std::string("Ollama response field ") + field + " exceeds the supported range.");
    return static_cast<int>(value.asUInt());
  }
  throw std::runtime_error(std::string("Ollama response field ") + field + " must be an integer.");
}

std::string tool_arguments_json(const Json::Value& arguments) {
  if (arguments.isNull())
    return "{}";
  if (arguments.isString())
    return arguments.asString();
  return compact_json(arguments);
}

}  // namespace

AIResponse OllamaProvider::generate(const AIRequest& request) {
  if (request.model.empty())
    throw std::invalid_argument("Ollama generation requires a non-empty model name.");

  const HttpEndpoint endpoint = parse_endpoint(base_url_);

  Json::Value payload(Json::objectValue);
  payload["model"] = request.model;
  payload["stream"] = false;
  Json::Value messages(Json::arrayValue);
  for (const auto& message : request.messages) {
    Json::Value encoded_message(Json::objectValue);
    encoded_message["role"] = message.role;
    encoded_message["content"] = message.content;
    messages.append(std::move(encoded_message));
  }
  payload["messages"] = std::move(messages);

  if (!request.tool_definitions_json.empty()) {
    Json::Value tools(Json::arrayValue);
    for (std::size_t index{}; index < request.tool_definitions_json.size(); ++index) {
      Json::Value definition;
      try {
        definition = parse_json(request.tool_definitions_json[index], "tool definition " + std::to_string(index + 1U));
      } catch (const std::exception& error) {
        throw std::invalid_argument(error.what());
      }
      if (!definition.isObject())
        throw std::invalid_argument("Ollama tool definition " + std::to_string(index + 1U) + " must be a JSON object.");
      tools.append(std::move(definition));
    }
    payload["tools"] = std::move(tools);
  }

  const std::string encoded_payload = compact_json(payload);
  const std::string http_request = "POST " + endpoint.chat_path + " HTTP/1.1\r\n" +
                                   "Host: " + endpoint.host_header + "\r\n" +
                                   "Content-Type: application/json\r\n" +
                                   "Accept: application/json\r\n" +
                                   "Connection: close\r\n" +
                                   "Content-Length: " + std::to_string(encoded_payload.size()) + "\r\n\r\n" +
                                   encoded_payload;

  const SocketHandle socket = connect_to(endpoint);
  send_all(socket.get(), http_request);
  const std::string raw_response = receive_http_response(socket.get());
  const auto body_offset = header_end_offset(raw_response);
  if (!body_offset)
    throw std::runtime_error("Ollama endpoint returned an HTTP response without a header terminator.");
  const HttpResponseMetadata metadata =
      parse_http_headers(std::string_view(raw_response.data(), *body_offset));
  const std::string body = response_body(raw_response, metadata, *body_offset);

  if (metadata.status_code < 200 || metadata.status_code >= 300) {
    const std::string detail = response_error_detail(body);
    throw std::runtime_error("Ollama endpoint returned HTTP " + std::to_string(metadata.status_code) +
                             (detail.empty() ? std::string{} : ": " + detail));
  }

  const Json::Value response = parse_json(body, "Ollama chat response");
  if (!response.isObject())
    throw std::runtime_error("Ollama chat response must be a JSON object.");
  if (response["error"].isString())
    throw std::runtime_error("Ollama returned an error: " + response["error"].asString());
  if (!response["message"].isObject())
    throw std::runtime_error("Ollama chat response is missing its message object.");

  const Json::Value& message = response["message"];
  if (!message["content"].isNull() && !message["content"].isString())
    throw std::runtime_error("Ollama chat response message content must be a string.");

  AIResponse result;
  result.content = message.get("content", "").asString();
  result.token_input = response_token_count(response, "prompt_eval_count");
  result.token_output = response_token_count(response, "eval_count");
  if (!response["model"].isNull() && !response["model"].isString())
    throw std::runtime_error("Ollama chat response model must be a string.");
  result.model = response.get("model", request.model).asString();

  const Json::Value& tool_calls = message["tool_calls"];
  if (!tool_calls.isNull()) {
    if (!tool_calls.isArray())
      throw std::runtime_error("Ollama chat response tool_calls must be an array.");
    result.tool_calls.reserve(tool_calls.size());
    for (Json::ArrayIndex index{}; index < tool_calls.size(); ++index) {
      const Json::Value& encoded_call = tool_calls[index];
      if (!encoded_call.isObject())
        throw std::runtime_error("Ollama chat response contains a non-object tool call.");
      const Json::Value& function = encoded_call["function"];
      if (!function.isObject() || !function["name"].isString() || function["name"].asString().empty())
        throw std::runtime_error("Ollama chat response contains a tool call without a function name.");

      ToolCall call;
      if (encoded_call["id"].isString() && !encoded_call["id"].asString().empty()) {
        call.id = encoded_call["id"].asString();
      } else if (function["id"].isString() && !function["id"].asString().empty()) {
        call.id = function["id"].asString();
      } else {
        // Ollama's native tool-call shape does not always expose an ID; retain a stable ID for local dispatch.
        call.id = "ollama-tool-" + std::to_string(static_cast<std::size_t>(index) + 1U);
      }
      call.name = function["name"].asString();
      call.arguments_json = tool_arguments_json(function["arguments"]);
      result.tool_calls.push_back(std::move(call));
    }
  }

  return result;
}

}  // namespace edu_ai::ai
