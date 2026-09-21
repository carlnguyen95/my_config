#include <cassert>
#include <chrono>
#include <filesystem>

#include "backend/repositories/Database.hpp"
#include "backend/repositories/AssessmentRepository.hpp"
#include "backend/repositories/CourseRepository.hpp"
#include "backend/repositories/DocumentChunkRepository.hpp"
#include "backend/repositories/DocumentRepository.hpp"
#include "backend/repositories/EnrollmentRepository.hpp"
#include "backend/repositories/LearningSessionRepository.hpp"
#include "backend/repositories/MessageRepository.hpp"
#include "backend/repositories/ProgressRepository.hpp"
#include "backend/repositories/QuestionRepository.hpp"
#include "backend/repositories/RoadmapRepository.hpp"
#include "backend/repositories/SubjectRepository.hpp"
#include "backend/repositories/TeacherConfigurationRepository.hpp"
#include "backend/repositories/UserRepository.hpp"
#include "tests/common/TestSupport.hpp"

#include <sqlite3.h>

namespace {
int count_rows(edu_ai::repositories::SqliteDatabase& database, const char* table) {
  const std::string sql = "SELECT COUNT(*) FROM " + std::string(table) + ";";
  sqlite3_stmt* statement = nullptr;
  const int prepare_status = sqlite3_prepare_v2(database.handle(), sql.c_str(), -1, &statement, nullptr);
  if (prepare_status != SQLITE_OK) {
    throw std::runtime_error("Cannot prepare SQL for COUNT: " + std::string(sqlite3_errmsg(database.handle())));
  }

  int count = 0;
  if (sqlite3_step(statement) == SQLITE_ROW) {
    count = sqlite3_column_int(statement, 0);
  }
  sqlite3_finalize(statement);
  return count;
}

std::string read_text(edu_ai::repositories::SqliteDatabase& database, const std::string& sql) {
  sqlite3_stmt* statement = nullptr;
  const int prepare_status = sqlite3_prepare_v2(database.handle(), sql.c_str(), -1, &statement, nullptr);
  if (prepare_status != SQLITE_OK) {
    throw std::runtime_error("Cannot prepare SQL for read: " + std::string(sqlite3_errmsg(database.handle())));
  }

  std::string value;
  if (sqlite3_step(statement) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(statement, 0);
    if (text != nullptr) {
      value = reinterpret_cast<const char*>(text);
    }
  }
  sqlite3_finalize(statement);
  return value;
}
}  // namespace

/**
 * @brief Verifies repository persistence.
 * @return Process exit code.
 */
int main() {
  edu_ai::tests::Suite suite("repository");
#undef assert
#define assert(EXPRESSION) CHECK(suite, (EXPRESSION))
  using namespace edu_ai::models;
  using namespace edu_ai::repositories;

  const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
  const auto path = std::filesystem::temp_directory_path() / ("edu_ai_repository_" + std::to_string(suffix) + ".db");
  SqliteDatabase database(path);
  database.initialize_schema(std::filesystem::path(EDU_AI_SOURCE_DIR) / "database/init.sql");
  SqliteSubjectRepository subjects(database);
  const auto subject = subjects.save({.name = "Networks", .description = "Computer networking fundamentals"});
  suite.scenario("Add subject id=" + std::to_string(subject.id) + " name='" + subject.name + "'");
  assert(subject.id > 0);
  suite.scenario("Find subject by name='Network' returns subject_id=" + std::to_string(subject.id));
  assert(subjects.find_by_name("Network").front().id == subject.id);

  suite.scenario("Raw SQL inserts and reads a subject row directly from SQLite");
  database.execute("INSERT INTO subjects(name, description) VALUES('DB smoke', 'Inserted by raw SQL');");
  assert(count_rows(database, "subjects") >= 2);
  assert(read_text(database, "SELECT name FROM subjects WHERE name='DB smoke';") == "DB smoke");
  database.execute("UPDATE subjects SET description = 'Updated by raw SQL' WHERE name='DB smoke';");
  assert(read_text(database, "SELECT description FROM subjects WHERE name='DB smoke';") == "Updated by raw SQL");
  const auto raw_sql_subject = subjects.find_by_name("DB smoke");
  assert(!raw_sql_subject.empty() && raw_sql_subject.front().description == "Updated by raw SQL");

  SqliteUserRepository users(database);
  auto teacher = users.create(
      {.name = "Teacher", .email = "teacher@example.test", .password_hash = "hash", .role = Role::Teacher});
  auto student = users.create(
      {.name = "Student", .email = "student@example.test", .password_hash = "hash", .role = Role::Student});
  suite.scenario("Add users teacher_id=" + std::to_string(teacher.id) + " student_id=" + std::to_string(student.id));
  assert(teacher.id > 0 && student.id > 0);
  suite.scenario("Find user by email='" + student.email + "' returns user_id=" + std::to_string(student.id));
  assert(users.find_by_email(student.email)->id == student.id);
  suite.scenario("Find user by name='Teach' returns teacher_id=" + std::to_string(teacher.id));
  assert(users.find_by_name("Teach").front().id == teacher.id);
  suite.scenario("Find users by status='active' returns count=2");
  assert(users.find_by_status("active").size() == 2);

  SqliteCourseRepository courses(database);
  SqliteEnrollmentRepository enrollments(database);
  auto course = courses.save({.subject_id = subject.id,
                              .name = "TCP",
                              .description = "TCP learning",
                              .teacher_id = teacher.id,
                              .answer_policy = AnswerPolicy::Guided});
  suite.scenario("Add course id=" + std::to_string(course.id) + " name='" + course.name + "'");
  assert(course.id > 0);
  suite.scenario("Find course by name='TC' returns course_id=" + std::to_string(course.id));
  assert(courses.find_by_name("TC").front().id == course.id);
  suite.scenario("Verify teacher_id=" + std::to_string(teacher.id) + " owns course_id=" + std::to_string(course.id));
  assert(courses.is_teacher_for(teacher.id, course.id));
  suite.scenario("Add enrollment user_id=" + std::to_string(student.id) + " course_id=" + std::to_string(course.id));
  assert(enrollments.create(student.id, course.id));
  suite.scenario("Reject duplicate enrollment user_id=" + std::to_string(student.id) +
                 " course_id=" + std::to_string(course.id));
  assert(!enrollments.create(student.id, course.id));
  suite.scenario("Verify enrollment user_id=" + std::to_string(student.id) + " course_id=" + std::to_string(course.id));
  assert(enrollments.exists(student.id, course.id));

  SqliteTeacherConfigurationRepository teacher_configurations(database);
  auto teacher_configuration =
      teacher_configurations.save({.teacher_id = teacher.id,
                                   .course_id = course.id,
                                   .policy_text = "Ask students for an attempt before explaining."});
  suite.scenario("Add teacher configuration id=" + std::to_string(teacher_configuration.id) +
                 " for teacher_id=" + std::to_string(teacher.id) + " course_id=" + std::to_string(course.id));
  assert(teacher_configuration.id > 0);
  suite.scenario("Find teacher configuration for teacher_id=" + std::to_string(teacher.id) +
                 " course_id=" + std::to_string(course.id));
  assert(teacher_configurations.find_for_teacher_course(teacher.id, course.id)->id == teacher_configuration.id);

  SqliteQuestionRepository questions(database);
  auto question = questions.save({.course_id = course.id,
                                  .created_by = teacher.id,
                                  .question = "Why congestion control?",
                                  .answer = "Prevent collapse",
                                  .hint = "Think about shared links",
                                  .difficulty = "medium",
                                  .question_type = "short_answer",
                                  .source = "teacher",
                                  .status = QuestionStatus::Approved});
  suite.scenario("Add approved question id=" + std::to_string(question.id) +
                 " for course_id=" + std::to_string(course.id));
  assert(question.id > 0);
  suite.scenario("Search approved questions in course_id=" + std::to_string(course.id) + " by content='congestion'");
  assert(questions.search(course.id, "congestion", true).front().id == question.id);
  suite.scenario("Find questions in course_id=" + std::to_string(course.id) + " by status=approved");
  assert(questions.find_by_status(course.id, QuestionStatus::Approved).front().id == question.id);
  suite.scenario("Find questions in course_id=" + std::to_string(course.id) + " by content='control'");
  assert(questions.find_by_content(course.id, "control").front().id == question.id);
  const auto removable_question = questions.save({.course_id = course.id,
                                                  .created_by = teacher.id,
                                                  .question = "Temporary question",
                                                  .answer = "Temporary answer",
                                                  .source = "teacher",
                                                  .status = QuestionStatus::Draft});
  suite.scenario("Add removable question id=" + std::to_string(removable_question.id) + " for course_id=" +
                 std::to_string(course.id));
  assert(removable_question.id > 0);
  suite.scenario("Remove question id=" + std::to_string(removable_question.id));
  assert(questions.remove(removable_question.id));
  suite.scenario("Verify removed question id=" + std::to_string(removable_question.id) + " no longer exists");
  assert(!questions.find_by_id(removable_question.id).has_value());

  SqliteLearningSessionRepository sessions(database);
  SqliteMessageRepository messages(database);
  auto session = sessions.save({.user_id = student.id, .course_id = course.id});
  auto message = messages.save({.session_id = session.id, .role = "user", .content = "Explain TCP", .model = ""});
  suite.scenario("Add message id=" + std::to_string(message.id) + " in session_id=" + std::to_string(session.id));
  assert(session.id > 0 && message.id > 0);
  suite.scenario("Read recent messages for session_id=" + std::to_string(session.id) + " with limit=8");
  assert(messages.recent_for_session(session.id, 8).front().id == message.id);
  suite.scenario("Find messages in session_id=" + std::to_string(session.id) + " by content='Explain'");
  assert(messages.find_by_content(session.id, "Explain").front().id == message.id);

  SqliteRoadmapRepository roadmaps(database);
  auto roadmap = roadmaps.save(
      {.user_id = student.id, .course_id = course.id, .title = "TCP roadmap", .content_json = "{\"topics\":[]}"});
  suite.scenario("Add roadmap id=" + std::to_string(roadmap.id) + " for student_id=" + std::to_string(student.id));
  assert(roadmap.id > 0);
  suite.scenario("Find roadmap for user_id=" + std::to_string(student.id) + " course_id=" + std::to_string(course.id));
  assert(roadmaps.find_for_user_course(student.id, course.id)->id == roadmap.id);
  suite.scenario("Find roadmap id=" + std::to_string(roadmap.id) + " by primary key");
  assert(roadmaps.find_by_id(roadmap.id)->id == roadmap.id);

  SqliteProgressRepository progress(database);
  auto item = progress.save({.user_id = student.id,
                             .course_id = course.id,
                             .topic = "TCP basics",
                             .status = ProgressStatus::Learning,
                             .progress_value = 0.25});
  suite.scenario("Add progress id=" + std::to_string(item.id) + " topic='" + item.topic + "'");
  assert(item.id > 0);
  suite.scenario("List progress for user_id=" + std::to_string(student.id) + " course_id=" + std::to_string(course.id));
  assert(progress.list(student.id, course.id).front().id == item.id);

  SqliteDocumentRepository documents(database);
  SqliteDocumentChunkRepository chunks(database);
  auto document = documents.save({.course_id = course.id, .title = "Course notes", .source_path = "notes.txt"});
  auto chunk = chunks.save(
      {.document_id = document.id, .chunk_index = 0, .content = "TCP provides reliable delivery.", .embedding = "[]"});
  suite.scenario("Add document id=" + std::to_string(document.id) + " and chunk_id=" + std::to_string(chunk.id));
  assert(document.id > 0 && chunk.id > 0);
  suite.scenario("List chunks for document_id=" + std::to_string(document.id));
  assert(chunks.list_for_document(document.id).front().id == chunk.id);
  suite.scenario("Find chunks for document_id=" + std::to_string(document.id) + " by content='reliable'");
  assert(chunks.find_by_content(document.id, "reliable").front().id == chunk.id);

  SqliteAssessmentRepository assessments(database);
  auto assessment = assessments.save({.user_id = student.id,
                                      .course_id = course.id,
                                      .message_id = message.id,
                                      .score = 0.8,
                                      .dimensions_json = "{}",
                                      .reasoning = "Good question",
                                      .model_name = "local",
                                      .review_status = "pending"});
  assessment.teacher_score = 0.9;
  assessment.teacher_feedback = "Strong causal framing";
  assessment.review_status = "reviewed";
  assessments.save(assessment);
  suite.scenario("Update assessment id=" + std::to_string(assessment.id) + " with teacher_score=0.9");
  assert(assessment.id > 0);
  suite.scenario("Find assessment by id=" + std::to_string(assessment.id) + " has status='reviewed'");
  assert(assessments.find_by_id(assessment.id)->review_status == "reviewed");
  suite.scenario("Find assessments in course_id=" + std::to_string(course.id) + " by status='reviewed'");
  assert(assessments.find_by_status(course.id, "reviewed").front().id == assessment.id);
  suite.scenario("List assessments for student_id=" + std::to_string(student.id) + " in course_id=" +
                 std::to_string(course.id));
  assert(assessments.list_for_user_course(student.id, course.id).front().id == assessment.id);

  const auto removable_course = courses.save({.subject_id = subject.id,
                                              .name = "Temporary course",
                                              .description = "Created only to test removal",
                                              .teacher_id = teacher.id,
                                              .answer_policy = AnswerPolicy::Guided});
  suite.scenario("Add removable course id=" + std::to_string(removable_course.id) + " name='" + removable_course.name +
                 "'");
  assert(removable_course.id > 0);
  suite.scenario("Remove course id=" + std::to_string(removable_course.id) + " name='" + removable_course.name + "'");
  assert(courses.remove(removable_course.id));
  suite.scenario("Verify removed course id=" + std::to_string(removable_course.id) + " no longer exists");
  assert(!courses.find_by_id(removable_course.id).has_value());
  return suite.finish();
}
