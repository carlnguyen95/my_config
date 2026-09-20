#include "backend/ai/PolicyEngine.hpp"

namespace edu_ai::ai {
/**
 * @brief Resolves the effective answer policy for the caller's role.
 * @param role Role of the requesting user.
 * @param course_policy Answer policy configured for the course.
 * @return Effective answer policy applied to the request.
 */
models::AnswerPolicy PolicyEngine::resolve_answer_policy(models::Role role, models::AnswerPolicy course_policy) const {
  return role == models::Role::Student ? course_policy : models::AnswerPolicy::FullAnswer;
}

/**
 * @brief Checks whether the effective policy permits a direct answer.
 * @param role Role of the requesting user.
 * @param effective_policy Policy resolved for the request.
 * @return True when a direct answer may be shown.
 */
bool PolicyEngine::can_view_direct_answer(models::Role role, models::AnswerPolicy effective_policy) const {
  return role != models::Role::Student || effective_policy == models::AnswerPolicy::FullAnswer;
}

/**
 * @brief Checks whether the role may manage question-bank content.
 * @param role Role to authorize.
 * @return True when the role may manage questions.
 */
bool PolicyEngine::can_manage_questions(models::Role role) const {
  return role == models::Role::Teacher || role == models::Role::Admin;
}

/**
 * @brief Checks whether the role may review a thinking assessment.
 * @param role Role to authorize.
 * @return True when the role may review assessments.
 */
bool PolicyEngine::can_review_assessment(models::Role role) const {
  return role == models::Role::Teacher || role == models::Role::Admin;
}
}  // namespace edu_ai::ai
