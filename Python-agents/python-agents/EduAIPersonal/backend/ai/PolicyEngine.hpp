#pragma once

#include "backend/models/Domain.hpp"

namespace edu_ai::ai {
class PolicyEngine {
 public:
  models::AnswerPolicy resolve_answer_policy(models::Role role, models::AnswerPolicy course_policy) const;
  bool can_view_direct_answer(models::Role role, models::AnswerPolicy effective_policy) const;
  bool can_manage_questions(models::Role role) const;
  bool can_review_assessment(models::Role role) const;
};
}  // namespace edu_ai::ai

