#pragma once

#include "backend/ai/AIProvider.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace edu_ai::ai {

/**
 * @brief Immutable snapshot of the routing and latency counters for one model tag.
 *
 * Latency includes every completed delegate call, including calls which throw. A
 * snapshot may be slightly internally inconsistent while requests are in flight,
 * but it is safe to obtain concurrently with generation.
 */
struct RoundRobinModelMetrics {
  std::string model;
  std::uint64_t requests{};
  std::uint64_t successes{};
  std::uint64_t failures{};
  std::uint64_t latency_samples{};
  std::chrono::nanoseconds total_latency{};
  std::chrono::nanoseconds minimum_latency{};
  std::chrono::nanoseconds maximum_latency{};
  double average_latency_ms{};
};

/**
 * @brief Extra routing metadata available without changing the AIProvider interface.
 *
 * AIProvider::generate() remains compatible with existing callers. Call
 * RoundRobinProvider::generate_routed() when the selected model, atomic ticket,
 * and delegate latency are needed by a caller. The contained AIResponse also has
 * its model field normalized to selected_model.
 */
struct RoundRobinResult {
  AIResponse response;
  std::string selected_model;
  std::uint64_t ticket{};
  std::chrono::nanoseconds provider_latency{};
};

/**
 * @brief Per-call event suitable for an external latency benchmark or telemetry sink.
 *
 * The observer runs on the calling thread after the delegate returns or throws.
 * Its exceptions are swallowed so telemetry cannot change request behavior.
 */
struct RoundRobinCompletion {
  std::uint64_t ticket{};
  std::string model;
  bool success{};
  double provider_latency_ms{};
};

/**
 * @brief Selects configured model tags in round-robin order before delegating generation.
 *
 * Empty tags are ignored. Duplicate nonempty tags are retained as additional
 * round-robin slots (which can be used as weights), while their metrics are
 * aggregated under the shared tag. The wrapped provider must itself support
 * concurrent generate() calls; this wrapper deliberately does not serialize
 * requests, so it can be used to measure real concurrent latency.
 */
class RoundRobinProvider final : public AIProvider {
 public:
  using CompletionObserver = std::function<void(const RoundRobinCompletion&)>;

  RoundRobinProvider(AIProvider& delegate, std::vector<std::string> model_tags);
  ~RoundRobinProvider() override;

  RoundRobinProvider(const RoundRobinProvider&) = delete;
  RoundRobinProvider& operator=(const RoundRobinProvider&) = delete;
  RoundRobinProvider(RoundRobinProvider&&) = delete;
  RoundRobinProvider& operator=(RoundRobinProvider&&) = delete;

  /**
   * @brief Routes one request and returns the ordinary AIProvider response.
   */
  AIResponse generate(const AIRequest& request) override;

  /**
   * @brief Routes one request and returns its scheduling ticket and measured delegate latency.
   */
  [[nodiscard]] RoundRobinResult generate_routed(const AIRequest& request);

  /**
   * @brief Replaces the optional, thread-safe observer used for completed calls.
   *
   * Pass an empty function to disable observation. The observer is copied before
   * invocation, so it may safely replace itself from inside its callback.
   */
  void set_completion_observer(CompletionObserver observer);

  /**
   * @brief Returns a thread-safe snapshot of metrics grouped by model tag.
   */
  [[nodiscard]] std::vector<RoundRobinModelMetrics> metrics() const;

  /**
   * @brief Returns the immutable configured routing slots, including intentional duplicates.
   */
  [[nodiscard]] const std::vector<std::string>& model_slots() const noexcept;

  /**
   * @brief Identifies this scheduling wrapper rather than the wrapped transport.
   */
  [[nodiscard]] std::string name() const override;

 private:
  struct ModelState;

  void record_completion(std::size_t model_index, std::chrono::nanoseconds latency, bool success) noexcept;
  void notify_completion(std::uint64_t ticket, const std::string& model, bool success,
                         std::chrono::nanoseconds latency) noexcept;

  AIProvider& delegate_;
  std::vector<std::string> model_slots_;
  std::vector<std::size_t> slot_model_indexes_;
  std::vector<std::string> metric_model_names_;
  std::vector<std::unique_ptr<ModelState>> model_states_;
  std::atomic<std::uint64_t> next_ticket_{0};

  mutable std::mutex observer_mutex_;
  CompletionObserver completion_observer_;
};

}  // namespace edu_ai::ai
