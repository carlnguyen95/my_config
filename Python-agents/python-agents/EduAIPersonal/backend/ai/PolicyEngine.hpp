#pragma once

#include "backend/models/Domain.hpp"

namespace edu_ai::ai {
class PolicyEngine {
 public:
  /// Resolves the answer policy after applying role-based overrides.
  models::AnswerPolicy resolve_answer_policy(models::Role role, models::AnswerPolicy course_policy) const;
  /// Reports whether the resolved policy permits a direct answer.
  bool can_view_direct_answer(models::Role role, models::AnswerPolicy effective_policy) const;
  /// Reports whether a role may manage the course question bank.
  bool can_manage_questions(models::Role role) const;
  /// Reports whether a role may review thinking assessments.
  bool can_review_assessment(models::Role role) const;
};
}  // namespace edu_ai::ai
