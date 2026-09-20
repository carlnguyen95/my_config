#include "backend/services/ProgressService.hpp"
#include "backend/services/ServiceSupport.hpp"

namespace edu_ai::services {
using namespace detail;

/**
 * @brief Lists visible progress.
 * @param actor_id Requester ID.
 * @param user_id Progress owner.
 * @param course_id Course ID.
 * @return Progress list or error.
 */
common::Result<std::vector<LearningProgress>> DefaultProgressService::list(Id actor_id, Id user_id, Id course_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<std::vector<LearningProgress>>();
  if (current->id != user_id && !can_manage_course(*current, courses_, course_id))
    return denied<std::vector<LearningProgress>>();
  if (current->id == user_id && !can_access_course(*current, courses_, enrollments_, course_id))
    return denied<std::vector<LearningProgress>>();
  return common::Result<std::vector<LearningProgress>>::success(progress_.list(user_id, course_id));
}

/**
 * @brief Validates and saves progress.
 * @param actor_id Requester ID.
 * @param progress Progress fields.
 * @return Saved progress or error.
 */
common::Result<LearningProgress> DefaultProgressService::update(Id actor_id, LearningProgress progress) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<LearningProgress>();
  if (current->id != progress.user_id && !can_manage_course(*current, courses_, progress.course_id))
    return denied<LearningProgress>();
  if (current->id == progress.user_id && !can_access_course(*current, courses_, enrollments_, progress.course_id))
    return denied<LearningProgress>();
  if (progress.topic.empty() || progress.progress_value < 0 || progress.progress_value > 1)
    return common::Result<LearningProgress>::failure(common::ErrorCode::InvalidRequest,
                                                     "Topic and progress value between 0 and 1 are required.");
  return common::Result<LearningProgress>::success(progress_.save(std::move(progress)));
}
}  // namespace edu_ai::services
