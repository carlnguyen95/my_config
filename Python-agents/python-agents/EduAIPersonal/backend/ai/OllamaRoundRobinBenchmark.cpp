#include "backend/ai/AIProvider.hpp"
#include "backend/ai/RoundRobinProvider.hpp"

#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <latch>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <json/json.h>

namespace {

struct BenchmarkOptions {
  std::filesystem::path config_path{"config/ai.json"};
  std::optional<std::string> base_url_override;
  std::optional<std::filesystem::path> output_path;
  std::string prompt{"Reply with exactly one word: pong."};
  std::size_t requests{10};
  std::size_t concurrency{10};
  bool warmup{};
  bool help{};
};

struct BenchmarkConfiguration {
  std::string base_url;
  std::vector<std::string> models;
};

struct BenchmarkRecord {
  std::uint64_t ticket{};
  std::string model;
  bool success{};
  double provider_latency_ms{};
  std::string error;
};

thread_local std::optional<edu_ai::ai::RoundRobinCompletion> completion_for_current_call;

[[noreturn]] void usage_error(const std::string& message) {
  throw std::invalid_argument(message + "\nRun with --help for usage.");
}

std::size_t parse_positive_count(std::string_view value, const std::string& option) {
  if (value.empty())
    usage_error(option + " requires a positive integer.");

  std::size_t parsed{};
  const auto [position, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
  if (error != std::errc{} || position != value.data() + value.size() || parsed == 0U)
    usage_error(option + " requires a positive integer.");
  return parsed;
}

std::string take_value(int& index, int argc, char* argv[], const std::string& option) {
  if (++index >= argc)
    usage_error(option + " requires a value.");
  return argv[index];
}

BenchmarkOptions parse_options(int argc, char* argv[]) {
  BenchmarkOptions options;

  for (int index = 1; index < argc; ++index) {
    const std::string_view argument = argv[index];
    if (argument == "--help" || argument == "-h") {
      options.help = true;
    } else if (argument == "--config") {
      options.config_path = take_value(index, argc, argv, "--config");
    } else if (argument == "--base-url") {
      options.base_url_override = take_value(index, argc, argv, "--base-url");
    } else if (argument == "--requests") {
      options.requests = parse_positive_count(take_value(index, argc, argv, "--requests"), "--requests");
    } else if (argument == "--concurrency") {
      options.concurrency = parse_positive_count(take_value(index, argc, argv, "--concurrency"), "--concurrency");
    } else if (argument == "--prompt") {
      options.prompt = take_value(index, argc, argv, "--prompt");
    } else if (argument == "--output") {
      options.output_path = take_value(index, argc, argv, "--output");
    } else if (argument == "--warmup") {
      options.warmup = true;
    } else {
      usage_error("Unknown argument: " + std::string(argument));
    }
  }

  return options;
}

void print_usage(std::ostream& output) {
  output << "Usage: ollama_round_robin_benchmark [options]\n"
            "\n"
            "Sends a simultaneous non-streaming burst to the configured local Ollama model pool.\n"
            "\n"
            "Options:\n"
            "  --config <path>        AI config path (default: config/ai.json)\n"
            "  --base-url <url>       Override ollama.base_url from the config\n"
            "  --requests <n>         Total requests in the burst (default: 10)\n"
            "  --concurrency <n>      Concurrent workers (default: 10)\n"
            "  --prompt <text>        User message sent in every request\n"
            "  --warmup               Send one unmeasured request to every configured model first\n"
            "  --output <csv-path>    Save one result row per dispatched request\n"
            "  --help, -h             Show this help\n";
}

BenchmarkConfiguration load_configuration(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input)
    throw std::runtime_error("Cannot open AI config: " + path.string());

  Json::Value root;
  Json::CharReaderBuilder parser;
  std::string errors;
  if (!Json::parseFromStream(parser, input, &root, &errors) || !root.isObject())
    throw std::runtime_error("Cannot parse AI config " + path.string() + ": " + errors);

  const Json::Value& ollama = root["ollama"];
  if (!ollama.isObject() || !ollama["base_url"].isString() || ollama["base_url"].asString().empty())
    throw std::runtime_error("AI config must contain a non-empty ollama.base_url.");

  const Json::Value& routing = ollama["routing"];
  if (!routing.isObject() || !routing["strategy"].isString() || routing["strategy"].asString() != "round_robin")
    throw std::runtime_error("AI config must set ollama.routing.strategy to round_robin.");
  const Json::Value& models = routing["models"];
  if (!models.isArray() || models.empty())
    throw std::runtime_error("AI config must contain at least one ollama.routing.models item.");

  BenchmarkConfiguration configuration{.base_url = ollama["base_url"].asString()};
  configuration.models.reserve(models.size());
  for (Json::ArrayIndex index{}; index < models.size(); ++index) {
    if (!models[index].isString() || models[index].asString().empty())
      throw std::runtime_error("Every ollama.routing.models item must be a non-empty string.");
    configuration.models.push_back(models[index].asString());
  }
  return configuration;
}

bool is_available_model(const std::string& configured_model, const std::vector<std::string>& installed_models) {
  if (std::find(installed_models.begin(), installed_models.end(), configured_model) != installed_models.end())
    return true;
  return configured_model.find(':') == std::string::npos &&
         std::find(installed_models.begin(), installed_models.end(), configured_model + ":latest") != installed_models.end();
}

void verify_configured_models(edu_ai::ai::OllamaProvider& provider, const std::vector<std::string>& configured_models) {
  const std::vector<std::string> installed_models = provider.list_models();
  std::vector<std::string> missing_models;
  for (const auto& model : configured_models) {
    if (!is_available_model(model, installed_models))
      missing_models.push_back(model);
  }
  if (missing_models.empty())
    return;

  std::string message{"Ollama is missing configured model tags: "};
  for (std::size_t index{}; index < missing_models.size(); ++index)
    message += (index == 0 ? "" : ", ") + missing_models[index];
  message += ". Installed: ";
  for (std::size_t index{}; index < installed_models.size(); ++index)
    message += (index == 0 ? "" : ", ") + installed_models[index];
  throw std::runtime_error(message);
}

double milliseconds(std::chrono::nanoseconds duration) {
  return static_cast<double>(duration.count()) / 1'000'000.0;
}

std::string csv_field(const std::string& value) {
  if (value.find_first_of(",\"\n\r") == std::string::npos)
    return value;

  std::string escaped;
  escaped.reserve(value.size() + 2U);
  escaped.push_back('"');
  for (const char character : value) {
    if (character == '"')
      escaped.push_back('"');
    escaped.push_back(character);
  }
  escaped.push_back('"');
  return escaped;
}

void write_csv(const std::filesystem::path& path, const std::vector<BenchmarkRecord>& records) {
  std::ofstream output(path);
  if (!output)
    throw std::runtime_error("Cannot write benchmark CSV: " + path.string());

  output << "ticket,model,success,provider_latency_ms,error\n";
  output << std::fixed << std::setprecision(3);
  for (const auto& record : records) {
    output << record.ticket << ',' << csv_field(record.model) << ',' << (record.success ? "true" : "false") << ','
           << record.provider_latency_ms << ',' << csv_field(record.error) << '\n';
  }
  if (!output)
    throw std::runtime_error("Failed while writing benchmark CSV: " + path.string());
}

void print_records(const std::vector<BenchmarkRecord>& records) {
  std::cout << "\nPer-request results\n";
  std::cout << std::fixed << std::setprecision(3);
  for (const auto& record : records) {
    std::cout << "  ticket=" << record.ticket << " model=" << record.model << " status="
              << (record.success ? "ok" : "error") << " provider_ms=" << record.provider_latency_ms;
    if (!record.error.empty())
      std::cout << " error=" << record.error;
    std::cout << '\n';
  }
}

void print_summary(const std::vector<edu_ai::ai::RoundRobinModelMetrics>& metrics, double burst_wall_ms,
                   std::size_t requests) {
  std::cout << "\nPer-model summary\n";
  std::cout << std::fixed << std::setprecision(3);
  for (const auto& metric : metrics) {
    std::cout << "  model=" << metric.model << " requests=" << metric.requests << " ok=" << metric.successes
              << " errors=" << metric.failures << " min_ms=" << milliseconds(metric.minimum_latency)
              << " mean_ms=" << metric.average_latency_ms << " max_ms=" << milliseconds(metric.maximum_latency) << '\n';
  }

  const double requests_per_second = burst_wall_ms <= 0.0 ? 0.0 : static_cast<double>(requests) * 1'000.0 / burst_wall_ms;
  std::cout << "\nBurst wall time: " << burst_wall_ms << " ms; throughput: " << requests_per_second << " req/s\n";
}

bool warm_models(edu_ai::ai::OllamaProvider& provider, const BenchmarkConfiguration& configuration,
                 const std::string& prompt) {
  bool all_succeeded = true;
  std::cout << "Warming each configured model (unmeasured)...\n";
  for (const auto& model : configuration.models) {
    const auto started_at = std::chrono::steady_clock::now();
    try {
      static_cast<void>(provider.generate({.messages = {{.role = "user", .content = prompt}}, .model = model}));
      const auto elapsed = std::chrono::steady_clock::now() - started_at;
      std::cout << "  " << model << ": ok (" << milliseconds(std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed))
                << " ms)\n";
    } catch (const std::exception& error) {
      all_succeeded = false;
      std::cerr << "  " << model << ": failed: " << error.what() << '\n';
    }
  }
  return all_succeeded;
}

int run(const BenchmarkOptions& options) {
  BenchmarkConfiguration configuration = load_configuration(options.config_path);
  if (options.base_url_override)
    configuration.base_url = *options.base_url_override;
  const std::size_t worker_count = std::min(options.requests, options.concurrency);

  std::cout << "Round-robin Ollama benchmark\n"
            << "  endpoint: " << configuration.base_url << '\n'
            << "  models: ";
  for (std::size_t index{}; index < configuration.models.size(); ++index)
    std::cout << (index == 0 ? "" : ", ") << configuration.models[index];
  std::cout << "\n  requests: " << options.requests << "\n  concurrent workers: " << worker_count << "\n";
  std::cout << std::flush;

  edu_ai::ai::OllamaProvider transport(configuration.base_url);
  verify_configured_models(transport, configuration.models);
  bool warmup_succeeded = true;
  if (options.warmup)
    warmup_succeeded = warm_models(transport, configuration, options.prompt);

  edu_ai::ai::RoundRobinProvider router(transport, configuration.models);
  std::mutex records_mutex;
  std::vector<BenchmarkRecord> records;
  records.reserve(options.requests);
  router.set_completion_observer([](const edu_ai::ai::RoundRobinCompletion& completion) {
    completion_for_current_call = completion;
  });

  const edu_ai::ai::AIRequest request{.messages = {{.role = "user", .content = options.prompt}}};
  std::atomic<std::size_t> next_request{0};
  std::atomic<bool> abort_workers{};
  std::latch workers_ready(static_cast<std::ptrdiff_t>(worker_count));
  std::promise<void> start_burst;
  const std::shared_future<void> start_signal = start_burst.get_future().share();
  std::vector<std::jthread> workers;
  workers.reserve(worker_count);
  try {
    for (std::size_t worker{}; worker < worker_count; ++worker) {
      workers.emplace_back([&, start_signal] {
        workers_ready.count_down();
        start_signal.wait();
        if (abort_workers.load(std::memory_order_relaxed))
          return;

        while (next_request.fetch_add(1, std::memory_order_relaxed) < options.requests) {
          completion_for_current_call.reset();
          try {
            const auto result = router.generate_routed(request);
            std::lock_guard lock(records_mutex);
            records.push_back({.ticket = result.ticket,
                               .model = result.selected_model,
                               .success = true,
                               .provider_latency_ms = milliseconds(result.provider_latency)});
          } catch (const std::exception& error) {
            std::lock_guard lock(records_mutex);
            if (completion_for_current_call) {
              records.push_back({.ticket = completion_for_current_call->ticket,
                                 .model = completion_for_current_call->model,
                                 .success = false,
                                 .provider_latency_ms = completion_for_current_call->provider_latency_ms,
                                 .error = error.what()});
            } else {
              records.push_back({.ticket = std::numeric_limits<std::uint64_t>::max(),
                                 .model = "unassigned",
                                 .success = false,
                                 .error = error.what()});
            }
          }
        }
      });
    }
  } catch (...) {
    abort_workers.store(true, std::memory_order_relaxed);
    start_burst.set_value();
    throw;
  }
  workers_ready.wait();
  const auto burst_started_at = std::chrono::steady_clock::now();
  start_burst.set_value();
  for (auto& worker : workers)
    worker.join();
  const auto burst_elapsed = std::chrono::steady_clock::now() - burst_started_at;

  std::sort(records.begin(), records.end(), [](const BenchmarkRecord& left, const BenchmarkRecord& right) {
    return left.ticket < right.ticket;
  });
  const auto metrics = router.metrics();
  print_records(records);
  print_summary(metrics, milliseconds(std::chrono::duration_cast<std::chrono::nanoseconds>(burst_elapsed)), options.requests);
  if (options.output_path) {
    write_csv(*options.output_path, records);
    std::cout << "CSV written to " << options.output_path->string() << '\n';
  }

  const bool complete = records.size() == options.requests;
  const bool all_succeeded = std::all_of(records.begin(), records.end(), [](const BenchmarkRecord& record) {
    return record.success;
  });
  if (!complete)
    std::cerr << "Benchmark internal error: expected " << options.requests << " results but captured " << records.size() << ".\n";
  return warmup_succeeded && complete && all_succeeded ? 0 : 1;
}

}  // namespace

int main(int argc, char* argv[]) {
  try {
    const BenchmarkOptions options = parse_options(argc, argv);
    if (options.help) {
      print_usage(std::cout);
      return 0;
    }
    return run(options);
  } catch (const std::exception& error) {
    std::cerr << "Benchmark failed: " << error.what() << '\n';
    return 2;
  }
}
