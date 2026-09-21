#include "backend/services/CourseService.hpp"
#include "backend/services/ServiceSupport.hpp"

namespace edu_ai::services {
using namespace detail;

/**
 * @brief Returns a course only when the actor has course access.
 * @param actor_id Authenticated requester ID.
 * @param course_id Course to load.
 * @return Course or an access/not-found failure.
 */
common::Result<Course> DefaultCourseService::get_course(Id actor_id, Id course_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<Course>();
  const auto course = courses_.find_by_id(course_id);
  if (!course)
    return common::Result<Course>::failure(common::ErrorCode::NotFound, "Course was not found.");
  return can_access_course(*current, courses_, enrollments_, course_id) ? common::Result<Course>::success(*course)
                                                                        : denied<Course>();
}

/**
 * @brief Creates a course while assigning ownership to its teacher actor.
 * @param actor_id Authenticated creator ID.
 * @param course Course fields to validate and persist.
 * @return Saved course or a validation/authorization failure.
 */
common::Result<Course> DefaultCourseService::create_course(Id actor_id, Course course) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<Course>();
  if (current->role != Role::Teacher && current->role != Role::Admin)
    return denied<Course>();
  if (course.name.empty() || course.subject_id == 0)
    return common::Result<Course>::failure(common::ErrorCode::InvalidRequest, "Course name and subject are required.");
  if (current->role == Role::Teacher)
    course.teacher_id = current->id;
  if (course.teacher_id == 0)
    return common::Result<Course>::failure(common::ErrorCode::InvalidRequest, "A course teacher is required.");
  try {
    return common::Result<Course>::success(courses_.save(std::move(course)));
  } catch (const std::exception&) {
    return common::Result<Course>::failure(common::ErrorCode::DatabaseError, "Unable to save course.");
  }
}

/**
 * @brief Updates a course after verifying teacher ownership.
 * @param actor_id Authenticated editor ID.
 * @param course Complete course fields including its ID.
 * @return Updated course or an authorization/validation failure.
 */
common::Result<Course> DefaultCourseService::update_course(Id actor_id, Course course) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<Course>();
  const auto existing = courses_.find_by_id(course.id);
  if (course.id == 0 || !existing)
    return common::Result<Course>::failure(common::ErrorCode::NotFound, "Course was not found.");
  if (!can_manage_course(*current, courses_, course.id))
    return denied<Course>();
  if (course.name.empty() || course.subject_id == 0)
    return common::Result<Course>::failure(common::ErrorCode::InvalidRequest, "Course name and subject are required.");
  if (current->role == Role::Teacher)
    course.teacher_id = current->id;
  else if (course.teacher_id == 0)
    course.teacher_id = existing->teacher_id;
  return common::Result<Course>::success(courses_.save(std::move(course)));
}

/**
 * @brief Enrolls the authenticated student in the requested course.
 * @param actor_id Authenticated student ID.
 * @param course_id Course to enroll in.
 * @return Whether an enrollment row was created, or an error.
 */
common::Result<bool> DefaultCourseService::enroll_self(Id actor_id, Id course_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<bool>();
  if (current->role != Role::Student)
    return denied<bool>();
  if (!courses_.find_by_id(course_id))
    return common::Result<bool>::failure(common::ErrorCode::NotFound, "Course was not found.");
  return common::Result<bool>::success(enrollments_.create(current->id, course_id));
}

/**
 * @brief Removes the authenticated student's enrollment from a course.
 * @param actor_id Authenticated student ID.
 * @param course_id Course to leave.
 * @return Whether an enrollment row was removed, or an error.
 */
common::Result<bool> DefaultCourseService::unenroll_self(Id actor_id, Id course_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<bool>();
  if (current->role != Role::Student)
    return denied<bool>();
  return common::Result<bool>::success(enrollments_.remove(current->id, course_id));
}

/**
 * @brief Searches course-learning content by keyword in an accessible course.
 * @param actor_id Authenticated requester ID.
 * @param course_id Course to search.
 * @param query Search text.
 * @return Matching course materials or an authorization/validation failure.
 */
common::Result<std::vector<DocumentChunk>> DefaultCourseService::find_info_in_courses(Id actor_id, Id course_id,
                                                                                   const std::vector<Id>& document_ids,
                                                                                   const std::string& query) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<std::vector<DocumentChunk>>();
  if (!can_access_course(*current, courses_, enrollments_, course_id))
    return denied<std::vector<DocumentChunk>>();
  if (!document_chunks_)
    return common::Result<std::vector<DocumentChunk>>::failure(common::ErrorCode::InvalidRequest,
                                                             "Course document search is not configured.");
  if (document_ids.empty())
    return common::Result<std::vector<DocumentChunk>>::failure(common::ErrorCode::InvalidRequest,
                                                             "At least one document_id is required.");
  if (query.empty())
    return common::Result<std::vector<DocumentChunk>>::failure(common::ErrorCode::InvalidRequest,
                                                             "A non-empty search query is required.");

  return common::Result<std::vector<DocumentChunk>>::success(
      document_chunks_->find_in_documents(course_id, document_ids, query));
}
}  // namespace edu_ai::services
