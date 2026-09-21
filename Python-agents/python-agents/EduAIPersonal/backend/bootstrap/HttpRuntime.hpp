#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "backend/repositories/CourseRepository.hpp"
#include "backend/repositories/Database.hpp"
#include "backend/repositories/DocumentChunkRepository.hpp"
#include "backend/repositories/DocumentRepository.hpp"
#include "backend/repositories/EnrollmentRepository.hpp"
#include "backend/repositories/TeacherConfigurationRepository.hpp"
#include "backend/repositories/UserRepository.hpp"
#include "backend/services/AuthService.hpp"
#include "backend/services/CourseService.hpp"

namespace edu_ai::bootstrap {

/// Application-scoped dependencies used by Drogon's automatically created HTTP controllers.
class HttpRuntime final {
 public:
  static void initialize(const std::filesystem::path& database_path, const std::filesystem::path& schema_path,
                         std::string jwt_secret);
  static HttpRuntime& instance();

  repositories::UserRepository& users() { return users_; }
  repositories::CourseRepository& courses() { return courses_; }
  repositories::DocumentRepository& documents() { return documents_; }
  repositories::DocumentChunkRepository& document_chunks() { return document_chunks_; }
  repositories::TeacherConfigurationRepository& teacher_configurations() { return teacher_configurations_; }
  services::AuthService& auth_service() { return auth_service_; }
  services::CourseService& course_service() { return course_service_; }

 private:
  HttpRuntime(const std::filesystem::path& database_path, const std::filesystem::path& schema_path,
              std::string jwt_secret);

  static std::unique_ptr<HttpRuntime> instance_;

  repositories::SqliteDatabase database_;
  repositories::SqliteUserRepository users_;
  repositories::SqliteCourseRepository courses_;
  repositories::SqliteEnrollmentRepository enrollments_;
  repositories::SqliteTeacherConfigurationRepository teacher_configurations_;
  repositories::SqliteDocumentRepository documents_;
  repositories::SqliteDocumentChunkRepository document_chunks_;
  services::DefaultAuthService auth_service_;
  services::DefaultCourseService course_service_;
};

}  // namespace edu_ai::bootstrap
