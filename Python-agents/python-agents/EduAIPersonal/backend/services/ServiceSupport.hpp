#pragma once

#include "backend/repositories/Repositories.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::services::detail {

/// Loads the authenticated actor from the repository.
inline std::optional<User> actor(repositories::UserRepository& users, Id actor_id) {
  return users.find_by_id(actor_id);
}

/// Reports whether the user has global administrator privileges.
inline bool is_admin(const User& user) {
  return user.role == Role::Admin;
}

/// Reports whether a user may read learning resources in a course.
inline bool can_access_course(const User& user, repositories::CourseRepository& courses,
                              repositories::EnrollmentRepository& enrollments, Id course_id) {
  return is_admin(user) || (user.role == Role::Teacher && courses.is_teacher_for(user.id, course_id)) ||
         (user.role == Role::Student && enrollments.exists(user.id, course_id));
}

/// Reports whether a user may modify resources belonging to a course.
inline bool can_manage_course(const User& user, repositories::CourseRepository& courses, Id course_id) {
  return is_admin(user) || (user.role == Role::Teacher && courses.is_teacher_for(user.id, course_id));
}

/// Creates a standard failure result for a missing authenticated actor.
template <typename T>
common::Result<T> missing_actor() {
  return common::Result<T>::failure(common::ErrorCode::Unauthenticated, "Authenticated user was not found.");
}

/// Creates a standard failure result for an unauthorized operation.
template <typename T>
common::Result<T> denied() {
  return common::Result<T>::failure(common::ErrorCode::PermissionDenied,
                                    "User is not allowed to perform this operation.");
}

}  // namespace edu_ai::services::detail
