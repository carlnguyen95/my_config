#include "backend/ai/PolicyEngine.hpp"

namespace edu_ai::ai {
models::AnswerPolicy PolicyEngine::resolve_answer_policy(models::Role role, models::AnswerPolicy course_policy) const {
  return role == models::Role::Student ? course_policy : models::AnswerPolicy::FullAnswer;
}
bool PolicyEngine::can_view_direct_answer(models::Role role, models::AnswerPolicy effective_policy) const {
  return role != models::Role::Student || effective_policy == models::AnswerPolicy::FullAnswer;
}
bool PolicyEngine::can_manage_questions(models::Role role) const { return role == models::Role::Teacher || role == models::Role::Admin; }
bool PolicyEngine::can_review_assessment(models::Role role) const { return role == models::Role::Teacher || role == models::Role::Admin; }
}  // namespace edu_ai::ai

