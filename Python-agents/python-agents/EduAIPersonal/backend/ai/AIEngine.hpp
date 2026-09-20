#pragma once

#include "backend/ai/AIProvider.hpp"
#include "backend/ai/PolicyEngine.hpp"
#include "backend/ai/PromptBuilder.hpp"
#include "backend/ai/ToolRegistry.hpp"
#include "backend/repositories/Repositories.hpp"

namespace edu_ai::ai {
struct LearningRequest {
  models::Id user_id{};
  models::Role role{models::Role::Student};
  models::Course course;
  std::string user_message;
  std::vector<AIMessage> history;
};

class AIEngine {
 public:
  AIEngine(AIProvider& provider, PolicyEngine& policies, PromptBuilder& prompts, ToolRegistry& tools,
           repositories::TeacherConfigurationRepository& teacher_configurations)
      : provider_(provider),
        policies_(policies),
        prompts_(prompts),
        tools_(tools),
        teacher_configurations_(teacher_configurations) {}

  /// Builds a policy-scoped prompt and delegates generation to the configured provider.
  AIResponse process(const LearningRequest& request);

 private:
  AIProvider& provider_;
  PolicyEngine& policies_;
  PromptBuilder& prompts_;
  ToolRegistry& tools_;
  repositories::TeacherConfigurationRepository& teacher_configurations_;
};
}  // namespace edu_ai::ai
