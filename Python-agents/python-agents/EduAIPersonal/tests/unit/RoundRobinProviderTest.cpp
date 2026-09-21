#include "backend/ai/RoundRobinProvider.hpp"
#include "tests/common/TestSupport.hpp"

#include <algorithm>
#include <barrier>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

class FakeProvider final : public edu_ai::ai::AIProvider {
 public:
  explicit FakeProvider(std::chrono::milliseconds delay = {}) : delay_(delay) {}

  edu_ai::ai::AIResponse generate(const edu_ai::ai::AIRequest& request) override {
    if (delay_.count() > 0)
      std::this_thread::sleep_for(delay_);

    {
      std::lock_guard lock(mutex_);
      received_models_.push_back(request.model);
      if (fail_next_) {
        fail_next_ = false;
        throw std::runtime_error("fake provider failure");
      }
    }

    return {.content = "response for " + request.model,
            .tool_calls = {},
            .token_input = 0,
            .token_output = 0,
            .model = "delegate-reported-model"};
  }

  std::string name() const override {
    return "fake";
  }

  void fail_next_request() {
    std::lock_guard lock(mutex_);
    fail_next_ = true;
  }

  std::vector<std::string> received_models() const {
    std::lock_guard lock(mutex_);
    return received_models_;
  }

 private:
  std::chrono::milliseconds delay_;
  mutable std::mutex mutex_;
  std::vector<std::string> received_models_;
  bool fail_next_{};
};

const edu_ai::ai::RoundRobinModelMetrics* find_metrics(
    const std::vector<edu_ai::ai::RoundRobinModelMetrics>& metrics, const std::string& model) {
  const auto found = std::find_if(metrics.begin(), metrics.end(), [&model](const auto& metric) {
    return metric.model == model;
  });
  return found == metrics.end() ? nullptr : &*found;
}

}  // namespace

/**
 * @brief Exercises deterministic and concurrent model selection by the round-robin provider.
 * @return Process exit code.
 */
int main() {
  edu_ai::tests::Suite suite("round_robin_provider");
  using edu_ai::ai::AIRequest;
  using edu_ai::ai::RoundRobinCompletion;
  using edu_ai::ai::RoundRobinProvider;

  suite.scenario("Reject a configuration that has no nonempty model tags");
  FakeProvider invalid_configuration_delegate;
  bool rejected_empty_configuration = false;
  try {
    static_cast<void>(RoundRobinProvider(invalid_configuration_delegate, {"", ""}));
  } catch (const std::invalid_argument&) {
    rejected_empty_configuration = true;
  }
  CHECK(suite, rejected_empty_configuration);

  FakeProvider sequential_delegate;
  RoundRobinProvider sequential_router(sequential_delegate, {"gemma4", "", "gemma4:12b", "qwen3:8b"});
  const std::vector<std::string> expected_cycle = {"gemma4", "gemma4:12b", "qwen3:8b",
                                                    "gemma4", "gemma4:12b", "qwen3:8b"};

  suite.scenario("Skip empty tags and repeat the configured model cycle in order");
  for (std::size_t index = 0; index < expected_cycle.size(); ++index) {
    const auto result = sequential_router.generate_routed(AIRequest{});
    CHECK(suite, result.ticket == index && result.selected_model == expected_cycle[index] &&
                     result.response.model == expected_cycle[index]);
  }
  CHECK(suite, sequential_delegate.received_models() == expected_cycle);

  suite.scenario("Aggregate successful calls and latency samples by selected model");
  const auto sequential_metrics = sequential_router.metrics();
  const auto* gemma4_metrics = find_metrics(sequential_metrics, "gemma4");
  const auto* gemma4_12b_metrics = find_metrics(sequential_metrics, "gemma4:12b");
  const auto* qwen3_metrics = find_metrics(sequential_metrics, "qwen3:8b");
  CHECK(suite, sequential_metrics.size() == 3 && gemma4_metrics != nullptr && gemma4_12b_metrics != nullptr &&
                   qwen3_metrics != nullptr);
  CHECK(suite, gemma4_metrics != nullptr && gemma4_metrics->requests == 2 && gemma4_metrics->successes == 2 &&
                   gemma4_metrics->failures == 0 && gemma4_metrics->latency_samples == 2);
  CHECK(suite, gemma4_12b_metrics != nullptr && gemma4_12b_metrics->requests == 2 &&
                   gemma4_12b_metrics->successes == 2 && gemma4_12b_metrics->failures == 0 &&
                   gemma4_12b_metrics->latency_samples == 2);
  CHECK(suite, qwen3_metrics != nullptr && qwen3_metrics->requests == 2 && qwen3_metrics->successes == 2 &&
                   qwen3_metrics->failures == 0 && qwen3_metrics->latency_samples == 2);

  suite.scenario("Record success and failure completion events without allowing observer exceptions to escape");
  FakeProvider observing_delegate;
  RoundRobinProvider observing_router(observing_delegate, {"gemma4"});
  std::mutex completion_mutex;
  std::vector<RoundRobinCompletion> completions;
  observing_router.set_completion_observer([&completion_mutex, &completions](const RoundRobinCompletion& completion) {
    std::lock_guard lock(completion_mutex);
    completions.push_back(completion);
    throw std::runtime_error("observer should be isolated");
  });
  const auto successful_result = observing_router.generate_routed(AIRequest{});
  observing_delegate.fail_next_request();
  bool provider_threw = false;
  try {
    static_cast<void>(observing_router.generate_routed(AIRequest{}));
  } catch (const std::runtime_error&) {
    provider_threw = true;
  }
  const auto observing_metrics = observing_router.metrics();
  CHECK(suite, successful_result.ticket == 0 && provider_threw && completions.size() == 2 &&
                   completions[0].success && !completions[1].success && completions[0].model == "gemma4" &&
                   completions[1].model == "gemma4" && completions[0].provider_latency_ms >= 0.0 &&
                   completions[1].provider_latency_ms >= 0.0);
  CHECK(suite, observing_metrics.size() == 1 && observing_metrics[0].requests == 2 &&
                   observing_metrics[0].successes == 1 && observing_metrics[0].failures == 1 &&
                   observing_metrics[0].latency_samples == 2);

  suite.scenario("Distribute ten simultaneous calls as 4, 3, and 3 across three model slots");
  FakeProvider concurrent_delegate(std::chrono::milliseconds{2});
  RoundRobinProvider concurrent_router(concurrent_delegate, {"gemma4", "gemma4:12b", "qwen3:8b"});
  constexpr std::size_t request_count = 10;
  std::barrier starting_gate(static_cast<std::ptrdiff_t>(request_count + 1));
  std::vector<std::optional<edu_ai::ai::RoundRobinResult>> results(request_count);
  std::vector<std::thread> workers;
  workers.reserve(request_count);
  for (std::size_t index = 0; index < request_count; ++index) {
    workers.emplace_back([&concurrent_router, &results, &starting_gate, index] {
      starting_gate.arrive_and_wait();
      results[index] = concurrent_router.generate_routed(AIRequest{});
    });
  }
  starting_gate.arrive_and_wait();
  for (auto& worker : workers)
    worker.join();

  std::map<std::string, std::size_t> selected_counts;
  std::set<std::uint64_t> tickets;
  for (const auto& result : results) {
    CHECK(suite, result.has_value());
    if (result) {
      ++selected_counts[result->selected_model];
      tickets.insert(result->ticket);
    }
  }
  CHECK(suite, selected_counts["gemma4"] == 4 && selected_counts["gemma4:12b"] == 3 &&
                   selected_counts["qwen3:8b"] == 3);
  CHECK(suite, tickets.size() == request_count && *tickets.begin() == 0 && *tickets.rbegin() == request_count - 1);

  return suite.finish();
}
