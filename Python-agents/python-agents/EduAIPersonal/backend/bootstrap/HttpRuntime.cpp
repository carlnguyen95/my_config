#include "backend/bootstrap/HttpRuntime.hpp"

#include <stdexcept>
#include <utility>

namespace edu_ai::bootstrap {

std::unique_ptr<HttpRuntime> HttpRuntime::instance_;

HttpRuntime::HttpRuntime(const std::filesystem::path& database_path, const std::filesystem::path& schema_path,
                         std::string jwt_secret)
    : database_(database_path),
      users_(database_),
      courses_(database_),
      enrollments_(database_),
      teacher_configurations_(database_),
      documents_(database_),
      document_chunks_(database_),
      auth_service_(users_, services::PasswordHasher{}, services::JwtTokenIssuer(std::move(jwt_secret))),
      course_service_(courses_, enrollments_, users_, document_chunks_) {
  database_.initialize_schema(schema_path);
}

void HttpRuntime::initialize(const std::filesystem::path& database_path, const std::filesystem::path& schema_path,
                             std::string jwt_secret) {
  if (instance_) {
    throw std::logic_error("HTTP runtime is already initialized.");
  }
  instance_ = std::unique_ptr<HttpRuntime>(new HttpRuntime(database_path, schema_path, std::move(jwt_secret)));
}

HttpRuntime& HttpRuntime::instance() {
  if (!instance_) {
    throw std::logic_error("HTTP runtime must be initialized before controllers are created.");
  }
  return *instance_;
}

}  // namespace edu_ai::bootstrap
