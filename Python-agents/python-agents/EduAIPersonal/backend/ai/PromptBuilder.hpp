#pragma once

#include <string>
#include <vector>

#include "backend/ai/AIProvider.hpp"
#include "backend/models/Domain.hpp"

namespace edu_ai::ai {
struct PromptContext {
  models::Role role{models::Role::Student};
  models::AnswerPolicy answer_policy{models::AnswerPolicy::Guided};
  std::string course_context;
  std::string teacher_policy;
  std::string learning_context;
  std::string retrieved_context;
  std::string question_bank_context;
  std::vector<AIMessage> history;
  std::string user_request;
};

class PromptBuilder {
 public:
  /// Builds ordered system, history, and user messages for learning assistance.
  std::vector<AIMessage> build_learning_messages(const PromptContext& context) const;
  /// Builds the constrained prompt used for thinking assessment.
  std::string build_assessment_prompt(const std::string& question) const;
};
}  // namespace edu_ai::ai
