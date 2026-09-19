#pragma once

#include <string>
#include <vector>

namespace edu_ai::ai {

struct AIMessage { std::string role; std::string content; };
struct ToolCall { std::string name; std::string arguments_json; };
struct AIRequest { std::vector<AIMessage> messages; std::vector<std::string> tool_definitions_json; std::string model; };
struct AIResponse { std::string content; std::vector<ToolCall> tool_calls; int token_input{}; int token_output{}; std::string model; };

class AIProvider {
 public:
  virtual ~AIProvider() = default;
  virtual AIResponse generate(const AIRequest& request) = 0;
  virtual std::string name() const = 0;
};

// Provider transport implementations are added during the first AI vertical slice.
class OllamaProvider final : public AIProvider {
 public:
  explicit OllamaProvider(std::string base_url) : base_url_(std::move(base_url)) {}
  AIResponse generate(const AIRequest& request) override;
  std::string name() const override { return "ollama"; }
 private: std::string base_url_;
};

class CloudProvider final : public AIProvider {
 public:
  AIResponse generate(const AIRequest& request) override;
  std::string name() const override { return "cloud"; }
};

}  // namespace edu_ai::ai

