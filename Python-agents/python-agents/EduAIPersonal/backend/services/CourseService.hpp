#pragma once

#include "backend/repositories/Repositories.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::services {
class DefaultCourseService final : public CourseService {
 public:
  DefaultCourseService(repositories::CourseRepository& courses, repositories::EnrollmentRepository& enrollments,
                       repositories::UserRepository& users)
      : courses_(courses), enrollments_(enrollments), users_(users), document_chunks_(nullptr) {}
  DefaultCourseService(repositories::CourseRepository& courses, repositories::EnrollmentRepository& enrollments,
                       repositories::UserRepository& users, repositories::DocumentChunkRepository& document_chunks)
      : courses_(courses), enrollments_(enrollments), users_(users), document_chunks_(&document_chunks) {}

  common::Result<Course> get_course(Id actor_id, Id course_id) override;
  common::Result<Course> create_course(Id actor_id, Course course) override;
  common::Result<Course> update_course(Id actor_id, Course course) override;
  common::Result<bool> enroll_self(Id actor_id, Id course_id) override;
  common::Result<bool> unenroll_self(Id actor_id, Id course_id) override;
  common::Result<std::vector<DocumentChunk>> find_info_in_courses(Id actor_id, Id course_id,
                                                                const std::vector<Id>& document_ids,
                                                                const std::string& query) override;

 private:
  repositories::CourseRepository& courses_;
  repositories::EnrollmentRepository& enrollments_;
  repositories::UserRepository& users_;
  repositories::DocumentChunkRepository* document_chunks_;
};
}  // namespace edu_ai::services
