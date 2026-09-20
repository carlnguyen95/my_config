#include "backend/ai/RoundRobinProvider.hpp"

#include <algorithm>
#include <atomic>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace edu_ai::ai {

struct RoundRobinProvider::ModelState {
  std::atomic<std::uint64_t> requests{0};
  std::atomic<std::uint64_t> successes{0};
  std::atomic<std::uint64_t> failures{0};
  std::atomic<std::uint64_t> latency_samples{0};
  std::atomic<std::uint64_t> total_latency_ns{0};
  std::atomic<std::uint64_t> minimum_latency_ns{std::numeric_limits<std::uint64_t>::max()};
  std::atomic<std::uint64_t> maximum_latency_ns{0};
};

namespace {

std::uint64_t latency_count(std::chrono::nanoseconds latency) noexcept {
  const auto count = latency.count();
  return count <= 0 ? 0 : static_cast<std::uint64_t>(count);
}

std::chrono::nanoseconds as_duration(std::uint64_t count) noexcept {
  using Rep = std::chrono::nanoseconds::rep;
  constexpr auto maximum = static_cast<std::uint64_t>(std::numeric_limits<Rep>::max());
  return std::chrono::nanoseconds{static_cast<Rep>(std::min(count, maximum))};
}

void record_minimum(std::atomic<std::uint64_t>& value, std::uint64_t candidate) noexcept {
  auto observed = value.load(std::memory_order_relaxed);
  while (candidate < observed &&
         !value.compare_exchange_weak(observed, candidate, std::memory_order_relaxed, std::memory_order_relaxed)) {
  }
}

void record_maximum(std::atomic<std::uint64_t>& value, std::uint64_t candidate) noexcept {
  auto observed = value.load(std::memory_order_relaxed);
  while (candidate > observed &&
         !value.compare_exchange_weak(observed, candidate, std::memory_order_relaxed, std::memory_order_relaxed)) {
  }
}

}  // namespace

RoundRobinProvider::RoundRobinProvider(AIProvider& delegate, std::vector<std::string> model_tags)
    : delegate_(delegate) {
  std::unordered_map<std::string, std::size_t> model_indexes;

  for (std::string& model : model_tags) {
    if (model.empty())
      continue;

    const auto [entry, inserted] = model_indexes.try_emplace(model, model_states_.size());
    if (inserted) {
      metric_model_names_.push_back(model);
      model_states_.push_back(std::make_unique<ModelState>());
    }

    model_slots_.push_back(std::move(model));
    slot_model_indexes_.push_back(entry->second);
  }

  if (model_slots_.empty())
    throw std::invalid_argument("RoundRobinProvider requires at least one nonempty model tag");
}

RoundRobinProvider::~RoundRobinProvider() = default;

AIResponse RoundRobinProvider::generate(const AIRequest& request) {
  return generate_routed(request).response;
}

RoundRobinResult RoundRobinProvider::generate_routed(const AIRequest& request) {
  const auto ticket = next_ticket_.fetch_add(1, std::memory_order_relaxed);
  const auto slot = static_cast<std::size_t>(ticket % model_slots_.size());
  const auto model_index = slot_model_indexes_[slot];
  const std::string& selected_model = model_slots_[slot];
  auto& state = *model_states_[model_index];

  AIRequest routed_request = request;
  routed_request.model = selected_model;
  state.requests.fetch_add(1, std::memory_order_relaxed);
  const auto started_at = std::chrono::steady_clock::now();

  AIResponse response;
  std::chrono::nanoseconds latency;
  try {
    response = delegate_.generate(routed_request);
    latency = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - started_at);
  } catch (...) {
    const auto latency =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - started_at);
    record_completion(model_index, latency, false);
    notify_completion(ticket, selected_model, false, latency);
    throw;
  }

  record_completion(model_index, latency, true);
  notify_completion(ticket, selected_model, true, latency);
  response.model = selected_model;
  return {.response = std::move(response),
          .selected_model = selected_model,
          .ticket = ticket,
          .provider_latency = latency};
}

void RoundRobinProvider::set_completion_observer(CompletionObserver observer) {
  std::lock_guard lock(observer_mutex_);
  completion_observer_ = std::move(observer);
}

std::vector<RoundRobinModelMetrics> RoundRobinProvider::metrics() const {
  std::vector<RoundRobinModelMetrics> snapshots;
  snapshots.reserve(model_states_.size());

  for (std::size_t index = 0; index < model_states_.size(); ++index) {
    const auto& state = *model_states_[index];
    const auto samples = state.latency_samples.load(std::memory_order_relaxed);
    const auto total = state.total_latency_ns.load(std::memory_order_relaxed);
    const auto minimum = state.minimum_latency_ns.load(std::memory_order_relaxed);

    snapshots.push_back({.model = metric_model_names_[index],
                         .requests = state.requests.load(std::memory_order_relaxed),
                         .successes = state.successes.load(std::memory_order_relaxed),
                         .failures = state.failures.load(std::memory_order_relaxed),
                         .latency_samples = samples,
                         .total_latency = as_duration(total),
                         .minimum_latency = samples == 0 ? std::chrono::nanoseconds{0} : as_duration(minimum),
                         .maximum_latency = as_duration(state.maximum_latency_ns.load(std::memory_order_relaxed)),
                         .average_latency_ms = samples == 0 ? 0.0
                                                             : static_cast<double>(total) /
                                                                   static_cast<double>(samples) / 1'000'000.0});
  }

  return snapshots;
}

const std::vector<std::string>& RoundRobinProvider::model_slots() const noexcept {
  return model_slots_;
}

std::string RoundRobinProvider::name() const {
  return "round_robin";
}

void RoundRobinProvider::record_completion(std::size_t model_index, std::chrono::nanoseconds latency,
                                           bool success) noexcept {
  auto& state = *model_states_[model_index];
  const auto elapsed_ns = latency_count(latency);
  state.total_latency_ns.fetch_add(elapsed_ns, std::memory_order_relaxed);
  record_minimum(state.minimum_latency_ns, elapsed_ns);
  record_maximum(state.maximum_latency_ns, elapsed_ns);

  if (success)
    state.successes.fetch_add(1, std::memory_order_relaxed);
  else
    state.failures.fetch_add(1, std::memory_order_relaxed);

  state.latency_samples.fetch_add(1, std::memory_order_relaxed);
}

void RoundRobinProvider::notify_completion(std::uint64_t ticket, const std::string& model, bool success,
                                           std::chrono::nanoseconds latency) noexcept {
  CompletionObserver observer;
  try {
    std::lock_guard lock(observer_mutex_);
    observer = completion_observer_;
  } catch (...) {
    return;
  }

  if (!observer)
    return;

  try {
    observer({.ticket = ticket,
              .model = model,
              .success = success,
              .provider_latency_ms = static_cast<double>(latency_count(latency)) / 1'000'000.0});
  } catch (...) {
    // Benchmark and telemetry callbacks must not affect the provider request.
  }
}

}  // namespace edu_ai::ai
