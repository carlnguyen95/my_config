#ifdef EDU_AI_ENABLE_DROGON

#include <filesystem>
#include <stdexcept>
#include <string>

#include <drogon/drogon.h>

#include "backend/controllers/Controllers.hpp"
#include "backend/repositories/CourseRepository.hpp"
#include "backend/repositories/Database.hpp"
#include "backend/repositories/DocumentChunkRepository.hpp"
#include "backend/repositories/DocumentRepository.hpp"
#include "backend/repositories/EnrollmentRepository.hpp"
#include "backend/repositories/LearningSessionRepository.hpp"
#include "backend/repositories/MessageRepository.hpp"
#include "backend/repositories/TeacherConfigurationRepository.hpp"
#include "backend/repositories/UserRepository.hpp"
#include "backend/services/AuthService.hpp"
#include "backend/services/CourseService.hpp"
#include "backend/services/QuestionService.hpp"
#include "backend/services/RoadmapService.hpp"
#include "backend/services/ProgressService.hpp"
#include "backend/services/AssessmentService.hpp"
#include "tests/common/TestSupport.hpp"

namespace {

using edu_ai::models::Role;
using edu_ai::repositories::SqliteCourseRepository;
using edu_ai::repositories::SqliteDatabase;
using edu_ai::repositories::SqliteDocumentChunkRepository;
using edu_ai::repositories::SqliteDocumentRepository;
using edu_ai::repositories::SqliteEnrollmentRepository;
using edu_ai::repositories::SqliteLearningSessionRepository;
using edu_ai::repositories::SqliteMessageRepository;
using edu_ai::repositories::SqliteTeacherConfigurationRepository;
using edu_ai::repositories::SqliteUserRepository;
using edu_ai::services::DefaultAuthService;
using edu_ai::services::DefaultCourseService;
using edu_ai::services::JwtTokenIssuer;
using edu_ai::services::PasswordHasher;

void assert_true(edu_ai::tests::Suite& suite, bool condition, const std::string& scenario) {
  suite.scenario(scenario);
  CHECK(suite, condition);
}

void assert_error(edu_ai::tests::Suite& suite, const Json::Value& response, const std::string& scenario) {
  suite.scenario(scenario);
  CHECK(suite, response.isMember("error") && response["error"].isObject());
}

}  // namespace

int main() {
  edu_ai::tests::Suite suite("drogon_api");

  const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
  const auto db_path = std::filesystem::temp_directory_path() / ("edu_ai_drogon_" + std::to_string(suffix) + ".db");
  SqliteDatabase database(db_path);
  const auto source = std::filesystem::path(EDU_AI_SOURCE_DIR);
  database.initialize_schema(source / "database/init.sql");
  database.initialize_schema(source / "database/test_db.sql");

  const PasswordHasher password_hasher;
  database.execute("UPDATE users SET password_hash = '" + password_hasher.hash("secret123") + "' WHERE id = 1;");

  SqliteUserRepository users(database);
  SqliteCourseRepository courses(database);
  SqliteEnrollmentRepository enrollments(database);
  SqliteTeacherConfigurationRepository teacher_configurations(database);
  SqliteDocumentRepository documents(database);
  SqliteDocumentChunkRepository document_chunks(database);
  SqliteMessageRepository messages(database);
  SqliteLearningSessionRepository sessions(database);

  DefaultAuthService auth_service(users, password_hasher,
                                  JwtTokenIssuer("test-secret-must-be-at-least-32-bytes"));
  DefaultCourseService course_service(courses, enrollments, users, document_chunks);

  edu_ai::controllers::AuthController auth_controller(auth_service, users);
  edu_ai::controllers::CourseController course_controller(course_service, courses, documents, document_chunks,
                                                       teacher_configurations);
  edu_ai::controllers::LearningController learning_controller(messages, sessions);

  auto login_response = auth_controller.login_data("minh.tran@eduai.vn", "secret123");
  assert_true(suite, login_response.isMember("token") && login_response.isMember("user"),
              "POST /api/auth/login returns token and user payload");

  auto invalid_login_response = auth_controller.login_data("minh.tran@eduai.vn", "wrong-password");
  assert_error(suite, invalid_login_response, "POST /api/auth/login rejects a wrong password");

  auto me_response = auth_controller.me_data(1);
  assert_true(suite, me_response.isMember("user") && me_response["user"]["id"].asInt64() == 1,
              "GET /api/auth/me returns the current user profile");

  auto course_list_response = course_controller.list_courses_data();
  assert_true(suite, course_list_response.isMember("courses") && !course_list_response["courses"].empty(),
              "GET /api/courses returns a non-empty course list");

  auto course_detail_response = course_controller.get_course_data(1, 1);
  assert_true(suite, course_detail_response.isMember("course") && course_detail_response["course"]["id"].asInt64() == 1,
              "GET /api/courses/{id} returns the selected course");

  auto unauthorized_course_response = course_controller.get_course_data(6, 2);
  assert_error(suite, unauthorized_course_response,
               "GET /api/courses/{id} rejects a student without access to the course");

  auto document_list_response = course_controller.list_documents_data(1);
  assert_true(suite, document_list_response.isMember("documents"),
              "GET /api/courses/{id}/documents returns the document list");

  auto chat_response = learning_controller.chat(6, 1, "hello from the API test");
  assert_true(suite, chat_response.isMember("answer") && chat_response["answer"].asString().find("placeholder") != std::string::npos,
              "POST /api/learning/chat returns a structured placeholder response");

  return suite.finish();
}

#else

int main() {
  return 0;
}

#endif
