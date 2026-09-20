#include <cassert>
#include <chrono>
#include <filesystem>

#include <sqlite3.h>

#include "backend/repositories/AssessmentRepository.hpp"
#include "backend/repositories/CourseRepository.hpp"
#include "backend/repositories/Database.hpp"
#include "backend/repositories/DocumentChunkRepository.hpp"
#include "backend/repositories/EnrollmentRepository.hpp"
#include "backend/repositories/MessageRepository.hpp"
#include "backend/repositories/ProgressRepository.hpp"
#include "backend/repositories/QuestionRepository.hpp"
#include "backend/repositories/RoadmapRepository.hpp"
#include "backend/repositories/TeacherConfigurationRepository.hpp"
#include "backend/repositories/UserRepository.hpp"
#include "backend/services/AssessmentService.hpp"
#include "backend/services/CourseService.hpp"
#include "backend/services/ProgressService.hpp"
#include "backend/services/QuestionService.hpp"
#include "backend/services/RoadmapService.hpp"
#include "tests/common/TestSupport.hpp"

namespace {
/**
 * @brief Counts seed-table records.
 * @param database Test database.
 * @param table Trusted table name.
 * @return Record count.
 */
int count(edu_ai::repositories::SqliteDatabase& database, const char* table) {
  sqlite3_stmt* statement = nullptr;
  const std::string sql = "SELECT COUNT(*) FROM " + std::string(table) + ";";
  assert(sqlite3_prepare_v2(database.handle(), sql.c_str(), -1, &statement, nullptr) == SQLITE_OK);
  assert(sqlite3_step(statement) == SQLITE_ROW);
  const int value = sqlite3_column_int(statement, 0);
  sqlite3_finalize(statement);
  return value;
}
}  // namespace

/**
 * @brief Verifies the seeded database.
 * @return Process exit code.
 */
int main() {
  edu_ai::tests::Suite suite("seed_database");
#undef assert
#define assert(EXPRESSION) CHECK(suite, (EXPRESSION))
  using namespace edu_ai::models;
  using namespace edu_ai::repositories;
  using namespace edu_ai::services;

  const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
  SqliteDatabase database(std::filesystem::temp_directory_path() / ("edu_ai_seed_" + std::to_string(suffix) + ".db"));
  const auto source = std::filesystem::path(EDU_AI_SOURCE_DIR);
  database.initialize_schema(source / "database/init.sql");
  database.initialize_schema(source / "database/test_db.sql");

  const char* tables[] = {"users",
                          "subjects",
                          "courses",
                          "teacher_configurations",
                          "enrollments",
                          "learning_sessions",
                          "messages",
                          "questions",
                          "documents",
                          "document_chunks",
                          "roadmaps",
                          "learning_progress",
                          "thinking_assessments"};
  for (const auto* table : tables) {
    suite.scenario(std::string("Seed table=") + table + " has record_count=20");
    assert(count(database, table) == 20);
  }

  SqliteUserRepository users(database);
  SqliteCourseRepository courses(database);
  SqliteTeacherConfigurationRepository teacher_configurations(database);
  SqliteEnrollmentRepository enrollments(database);
  SqliteQuestionRepository questions(database);
  SqliteMessageRepository messages(database);
  SqliteDocumentChunkRepository chunks(database);
  SqliteRoadmapRepository roadmaps(database);
  SqliteProgressRepository progress(database);
  SqliteAssessmentRepository assessments(database);

  suite.scenario("Find users by name='Student' returns count=15");
  assert(users.find_by_name("Student").size() == 15);
  suite.scenario("Find users by status='inactive' returns count=1");
  assert(users.find_by_status("inactive").size() == 1);
  suite.scenario("Find courses by name='Course' returns count=20");
  assert(courses.find_by_name("Course").size() == 20);
  suite.scenario("Find teacher policy for teacher_id=1 course_id=1");
  assert(teacher_configurations.find_for_teacher_course(1, 1)->policy_text == "Seed teacher policy for course 1");
  suite.scenario("Find questions in course_id=1 by status=draft returns count=1");
  assert(questions.find_by_status(1, QuestionStatus::Draft).size() == 1);
  suite.scenario("Find questions in course_id=1 by content='topic' returns count=1");
  assert(questions.find_by_content(1, "topic").size() == 1);
  suite.scenario("Find messages in session_id=1 by content='Seed message' returns count=1");
  assert(messages.find_by_content(1, "Seed message").size() == 1);
  suite.scenario("Find chunks in document_id=1 by content='topic' returns count=1");
  assert(chunks.find_by_content(1, "topic").size() == 1);
  suite.scenario("Find assessments in course_id=1 by status='pending' returns count=1");
  assert(assessments.find_by_status(1, "pending").size() == 1);

  DefaultCourseService course_service(courses, enrollments, users);
  DefaultQuestionService question_service(questions, courses, enrollments, users);
  DefaultRoadmapService roadmap_service(roadmaps, courses, enrollments, users);
  DefaultProgressService progress_service(progress, courses, enrollments, users);
  DefaultAssessmentService assessment_service(assessments, courses, enrollments, users);

  suite.scenario("Student user_id=6 can read enrolled course_id=1");
  assert(course_service.get_course(6, 1).ok());
  suite.scenario("Student user_id=6 cannot read draft question_id=1");
  assert(!question_service.find(6, 1).ok());
  suite.scenario("Teacher user_id=1 approves question_id=1");
  assert(question_service.approve(1, 1).ok());
  suite.scenario("Student user_id=6 can read approved question_id=1");
  assert(question_service.find(6, 1).ok());
  suite.scenario("Student user_id=6 reads own roadmap for course_id=1");
  assert(roadmap_service.get(6, 6, 1).ok());
  suite.scenario("Teacher user_id=1 reads progress of student_id=6 in course_id=1");
  assert(progress_service.list(1, 6, 1).ok());
  suite.scenario("Teacher user_id=1 reviews assessment_id=1 with score=0.8");
  assert(assessment_service
             .review(1, {.id = 1, .teacher_score = 0.8, .teacher_feedback = "Verified", .review_status = "reviewed"})
             .ok());
  return suite.finish();
}
