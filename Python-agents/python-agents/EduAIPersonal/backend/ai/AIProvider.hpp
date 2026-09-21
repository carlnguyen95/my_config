#pragma once

#include <string>
#include <vector>

namespace edu_ai::ai {

struct AIMessage {
  std::string role;
  std::string content;
};

struct ToolCall {
  std::string id;
  std::string name;
  std::string arguments_json;
};

struct AIRequest {
  std::vector<AIMessage> messages;
  std::vector<std::string> tool_definitions_json;
  std::string model;
};

struct AIResponse {
  std::string content;
  std::vector<ToolCall> tool_calls;
  int token_input{};
  int token_output{};
  std::string model;
};

class AIProvider {
 public:
  virtual ~AIProvider() = default;
  /// Generates one provider response for the assembled AI request.
  virtual AIResponse generate(const AIRequest& request) = 0;
  /// Returns the stable provider name used in telemetry and configuration.
  virtual std::string name() const = 0;
};

// Provider transport implementations are added during the first AI vertical slice.
class OllamaProvider final : public AIProvider {
 public:
  explicit OllamaProvider(std::string base_url) : base_url_(std::move(base_url)) {}

  /// Sends a request to the configured Ollama endpoint.
  AIResponse generate(const AIRequest& request) override;

  /// Lists model tags currently available from the configured Ollama endpoint.
  std::vector<std::string> list_models() const;

  /// Identifies this provider implementation.
  std::string name() const override {
    return "ollama";
  }

 private:
  std::string base_url_;
};

class CloudProvider final : public AIProvider {
 public:
  /// Sends a request to the configured cloud provider.
  AIResponse generate(const AIRequest& request) override;

  /// Identifies this provider implementation.
  std::string name() const override {
    return "cloud";
  }
};

}  // namespace edu_ai::ai
