#include "backend/services/TeacherConfigurationService.hpp"

#include "backend/services/ServiceSupport.hpp"

namespace edu_ai::services {
using namespace detail;

/**
 * @brief Returns a teacher course policy.
 * @param actor_id Requester ID.
 * @param teacher_id Policy owner.
 * @param course_id Course ID.
 * @return Configuration or error.
 */
common::Result<TeacherConfiguration> DefaultTeacherConfigurationService::get(Id actor_id, Id teacher_id, Id course_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<TeacherConfiguration>();
  if (current->id != teacher_id && !is_admin(*current))
    return denied<TeacherConfiguration>();
  if (!can_manage_course(*current, courses_, course_id))
    return denied<TeacherConfiguration>();
  const auto configuration = configurations_.find_for_teacher_course(teacher_id, course_id);
  return configuration ? common::Result<TeacherConfiguration>::success(*configuration)
                       : common::Result<TeacherConfiguration>::failure(common::ErrorCode::NotFound,
                                                                       "Teacher configuration was not found.");
}

/**
 * @brief Validates and saves a teacher policy.
 * @param actor_id Requester ID.
 * @param configuration Policy fields.
 * @return Saved configuration or error.
 */
common::Result<TeacherConfiguration> DefaultTeacherConfigurationService::save(Id actor_id,
                                                                              TeacherConfiguration configuration) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<TeacherConfiguration>();
  if (current->id != configuration.teacher_id && !is_admin(*current))
    return denied<TeacherConfiguration>();
  if (!can_manage_course(*current, courses_, configuration.course_id))
    return denied<TeacherConfiguration>();
  const auto course = courses_.find_by_id(configuration.course_id);
  if (!course || course->teacher_id != configuration.teacher_id)
    return common::Result<TeacherConfiguration>::failure(common::ErrorCode::InvalidRequest,
                                                         "Teacher configuration must belong to the course teacher.");
  if (configuration.policy_text.empty())
    return common::Result<TeacherConfiguration>::failure(common::ErrorCode::InvalidRequest,
                                                         "Teacher policy text is required.");
  if (configuration.id == 0) {
    const auto existing = configurations_.find_for_teacher_course(configuration.teacher_id, configuration.course_id);
    if (existing)
      configuration.id = existing->id;
  }
  return common::Result<TeacherConfiguration>::success(configurations_.save(std::move(configuration)));
}

}  // namespace edu_ai::services
