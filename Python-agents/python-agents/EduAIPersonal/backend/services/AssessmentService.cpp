#include "backend/services/AssessmentService.hpp"
#include "backend/services/ServiceSupport.hpp"

namespace edu_ai::services {
using namespace detail;

/**
 * @brief Lists the authenticated student's assessments in an accessible course.
 * @param actor_id Authenticated student ID.
 * @param course_id Course that owns the assessments.
 * @return The student's assessments or an authorization failure.
 */
common::Result<std::vector<ThinkingAssessment>> DefaultAssessmentService::list_for_self(Id actor_id, Id course_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<std::vector<ThinkingAssessment>>();
  if (!can_access_course(*current, courses_, enrollments_, course_id))
    return denied<std::vector<ThinkingAssessment>>();
  return common::Result<std::vector<ThinkingAssessment>>::success(assessments_.list_for_user_course(current->id, course_id));
}

/**
 * @brief Lists assessments after verifying that the actor manages the course.
 * @param actor_id Authenticated requester ID.
 * @param course_id Course that owns the assessments.
 * @return Assessments or an authorization failure.
 */
common::Result<std::vector<ThinkingAssessment>> DefaultAssessmentService::list_for_course(Id actor_id, Id course_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<std::vector<ThinkingAssessment>>();
  if (!can_manage_course(*current, courses_, course_id))
    return denied<std::vector<ThinkingAssessment>>();
  return common::Result<std::vector<ThinkingAssessment>>::success(assessments_.list_for_course(course_id));
}

/**
 * @brief Saves a teacher review after validating ownership and score boundaries.
 * @param actor_id Authenticated reviewer ID.
 * @param assessment Assessment ID and teacher review fields.
 * @return Updated assessment or a validation/authorization failure.
 */
common::Result<ThinkingAssessment> DefaultAssessmentService::review(Id actor_id, ThinkingAssessment assessment) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<ThinkingAssessment>();
  auto existing = assessments_.find_by_id(assessment.id);
  if (!existing)
    return common::Result<ThinkingAssessment>::failure(common::ErrorCode::NotFound, "Assessment was not found.");
  if (!can_manage_course(*current, courses_, existing->course_id))
    return denied<ThinkingAssessment>();
  if (!assessment.teacher_score || *assessment.teacher_score < 0 || *assessment.teacher_score > 1)
    return common::Result<ThinkingAssessment>::failure(common::ErrorCode::InvalidRequest,
                                                       "Teacher score between 0 and 1 is required.");
  existing->teacher_score = assessment.teacher_score;
  existing->teacher_feedback = assessment.teacher_feedback;
  existing->review_status = assessment.review_status.empty() ? "reviewed" : assessment.review_status;
  return common::Result<ThinkingAssessment>::success(assessments_.save(std::move(*existing)));
}

/**
 * @brief Creates an assessment in a teacher-managed course.
 * @param actor_id Authenticated teacher ID.
 * @param assessment New assessment fields.
 * @return Saved assessment or an error.
 */
common::Result<ThinkingAssessment> DefaultAssessmentService::create(Id actor_id, ThinkingAssessment assessment) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<ThinkingAssessment>();
  if (!can_manage_course(*current, courses_, assessment.course_id) || assessment.user_id == 0 || assessment.message_id == 0 ||
      assessment.score < 0 || assessment.score > 1)
    return common::Result<ThinkingAssessment>::failure(common::ErrorCode::InvalidRequest,
                                                       "Assessment create request is invalid or not allowed.");
  assessment.id = 0;
  return common::Result<ThinkingAssessment>::success(assessments_.save(std::move(assessment)));
}
}  // namespace edu_ai::services
