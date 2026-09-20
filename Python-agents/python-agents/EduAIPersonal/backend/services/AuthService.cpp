#include "backend/services/AuthService.hpp"

#include <array>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

namespace edu_ai::services {
namespace {
constexpr int kPbkdf2Iterations = 210000;
constexpr std::size_t kSaltBytes = 16;
constexpr std::size_t kHashBytes = 32;

/**
 * @brief Encodes binary data as lower-case hexadecimal text.
 * @param bytes Binary input.
 * @param length Input length.
 * @return Hexadecimal text.
 */
std::string to_hex(const unsigned char* bytes, std::size_t length) {
  static constexpr char digits[] = "0123456789abcdef";
  std::string output(length * 2, '0');
  for (std::size_t i = 0; i < length; ++i) {
    output[i * 2] = digits[bytes[i] >> 4];
    output[i * 2 + 1] = digits[bytes[i] & 0x0f];
  }
  return output;
}

/**
 * @brief Decodes hexadecimal text.
 * @param value Encoded text.
 * @return Decoded bytes or no value.
 */
std::optional<std::vector<unsigned char>> from_hex(std::string_view value) {
  if (value.size() % 2 != 0)
    return std::nullopt;
  auto nibble = [](char c) -> int {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    return -1;
  };
  std::vector<unsigned char> result(value.size() / 2);
  for (std::size_t i = 0; i < result.size(); ++i) {
    const auto high = nibble(value[i * 2]);
    const auto low = nibble(value[i * 2 + 1]);
    if (high < 0 || low < 0)
      return std::nullopt;
    result[i] = static_cast<unsigned char>((high << 4) | low);
  }
  return result;
}

/**
 * @brief Encodes binary data as URL-safe Base64.
 * @param data Binary input.
 * @param length Input length.
 * @return Encoded text.
 */
std::string base64url(const unsigned char* data, std::size_t length) {
  std::string encoded(4 * ((length + 2) / 3), '\0');
  const auto written =
      EVP_EncodeBlock(reinterpret_cast<unsigned char*>(encoded.data()), data, static_cast<int>(length));
  encoded.resize(written);
  for (auto& c : encoded) {
    if (c == '+')
      c = '-';
    else if (c == '/')
      c = '_';
  }
  while (!encoded.empty() && encoded.back() == '=') encoded.pop_back();
  return encoded;
}

/**
 * @brief Encodes a string as URL-safe Base64.
 * @param value Text input.
 * @return Encoded text.
 */
std::string base64url(const std::string& value) {
  return base64url(reinterpret_cast<const unsigned char*>(value.data()), value.size());
}

/**
 * @brief Converts a role to JWT claim text.
 * @param role Domain role.
 * @return Claim value.
 */
std::string role_name(models::Role role) {
  return role == models::Role::Teacher ? "teacher" : role == models::Role::Admin ? "admin" : "student";
}
}  // namespace

/**
 * @brief Derives a salted PBKDF2-SHA256 hash.
 * @param password Plaintext password.
 * @return Encoded password hash.
 */
std::string PasswordHasher::hash(const std::string& password) const {
  if (password.size() < 8)
    throw std::invalid_argument("Password must contain at least 8 characters.");
  std::array<unsigned char, kSaltBytes> salt{};
  std::array<unsigned char, kHashBytes> derived{};
  if (RAND_bytes(salt.data(), salt.size()) != 1 ||
      PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()), salt.data(), salt.size(),
                        kPbkdf2Iterations, EVP_sha256(), derived.size(), derived.data()) != 1) {
    throw std::runtime_error("Password hashing failed.");
  }
  return "pbkdf2-sha256$" + std::to_string(kPbkdf2Iterations) + "$" + to_hex(salt.data(), salt.size()) + "$" +
         to_hex(derived.data(), derived.size());
}

/**
 * @brief Verifies a password against a stored hash.
 * @param password Plaintext password.
 * @param encoded_hash Stored hash.
 * @return True when valid.
 */
bool PasswordHasher::verify(const std::string& password, const std::string& encoded_hash) const {
  const auto first = encoded_hash.find('$');
  const auto second = encoded_hash.find('$', first == std::string::npos ? first : first + 1);
  const auto third = encoded_hash.find('$', second == std::string::npos ? second : second + 1);
  if (first == std::string::npos || second == std::string::npos || third == std::string::npos ||
      encoded_hash.substr(0, first) != "pbkdf2-sha256")
    return false;
  int iterations{};
  try {
    iterations = std::stoi(encoded_hash.substr(first + 1, second - first - 1));
  } catch (...) {
    return false;
  }
  const auto salt = from_hex(std::string_view(encoded_hash).substr(second + 1, third - second - 1));
  const auto expected = from_hex(std::string_view(encoded_hash).substr(third + 1));
  if (!salt || !expected || iterations < 1 || expected->empty())
    return false;
  std::vector<unsigned char> actual(expected->size());
  if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()), salt->data(),
                        static_cast<int>(salt->size()), iterations, EVP_sha256(), static_cast<int>(actual.size()),
                        actual.data()) != 1)
    return false;
  return CRYPTO_memcmp(actual.data(), expected->data(), actual.size()) == 0;
}

/**
 * @brief Produces a signed JWT.
 * @param user Authenticated user.
 * @return Signed access token.
 */
std::string JwtTokenIssuer::issue(const models::User& user) const {
  if (secret_.size() < 32)
    throw std::invalid_argument("JWT signing secret must be at least 32 characters.");
  const auto header = base64url("{\"alg\":\"HS256\",\"typ\":\"JWT\"}");
  const auto expires_at = std::time(nullptr) + 60 * 60 * 8;
  const auto payload = base64url("{\"sub\":" + std::to_string(user.id) + ",\"role\":\"" + role_name(user.role) +
                                 "\",\"exp\":" + std::to_string(expires_at) + "}");
  const auto unsigned_token = header + "." + payload;
  std::array<unsigned char, EVP_MAX_MD_SIZE> signature{};
  unsigned int signature_length{};
  if (HMAC(EVP_sha256(), secret_.data(), static_cast<int>(secret_.size()),
           reinterpret_cast<const unsigned char*>(unsigned_token.data()), unsigned_token.size(), signature.data(),
           &signature_length) == nullptr)
    throw std::runtime_error("JWT signing failed.");
  return unsigned_token + "." + base64url(signature.data(), signature_length);
}

/**
 * @brief Registers a non-admin account.
 * @param name Display name.
 * @param email Login email.
 * @param password Plaintext password.
 * @param role Requested role.
 * @return Saved user or error.
 */
common::Result<User> DefaultAuthService::register_user(const std::string& name, const std::string& email,
                                                       const std::string& password, Role role) {
  if (name.empty() || email.empty() || email.find('@') == std::string::npos)
    return common::Result<User>::failure(common::ErrorCode::InvalidRequest, "Name and a valid email are required.");
  if (role == Role::Admin)
    return common::Result<User>::failure(common::ErrorCode::PermissionDenied,
                                         "Admin accounts cannot be self-registered.");
  if (users_.find_by_email(email))
    return common::Result<User>::failure(common::ErrorCode::Conflict, "Email is already registered.");
  try {
    return common::Result<User>::success(
        users_.create({.name = name, .email = email, .password_hash = hasher_.hash(password), .role = role}));
  } catch (const std::invalid_argument& error) {
    return common::Result<User>::failure(common::ErrorCode::InvalidRequest, error.what());
  } catch (const std::exception&) {
    return common::Result<User>::failure(common::ErrorCode::DatabaseError, "Unable to create user.");
  }
}

/**
 * @brief Authenticates an active user.
 * @param email Login email.
 * @param password Plaintext password.
 * @return Access token or error.
 */
common::Result<std::string> DefaultAuthService::login(const std::string& email, const std::string& password) {
  try {
    const auto user = users_.find_by_email(email);
    if (!user || !hasher_.verify(password, user->password_hash))
      return common::Result<std::string>::failure(common::ErrorCode::Unauthenticated, "Invalid email or password.");
    if (user->status != "active")
      return common::Result<std::string>::failure(common::ErrorCode::PermissionDenied, "Account is not active.");
    return common::Result<std::string>::success(issuer_.issue(*user));
  } catch (const std::exception&) {
    return common::Result<std::string>::failure(common::ErrorCode::DatabaseError, "Unable to complete login.");
  }
}
}  // namespace edu_ai::services
