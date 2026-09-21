#include "backend/ai/ToolRegistry.hpp"
#include "backend/repositories/AssessmentRepository.hpp"
#include "backend/repositories/CourseRepository.hpp"
#include "backend/repositories/Database.hpp"
#include "backend/repositories/DocumentChunkRepository.hpp"
#include "backend/repositories/EnrollmentRepository.hpp"
#include "backend/repositories/LearningSessionRepository.hpp"
#include "backend/repositories/MessageRepository.hpp"
#include "backend/repositories/ProgressRepository.hpp"
#include "backend/repositories/QuestionRepository.hpp"
#include "backend/repositories/RoadmapRepository.hpp"
#include "backend/repositories/SubjectRepository.hpp"
#include "backend/repositories/TeacherConfigurationRepository.hpp"
#include "backend/repositories/UserRepository.hpp"
#include "backend/services/AssessmentService.hpp"
#include "backend/services/CourseService.hpp"
#include "backend/services/ProgressService.hpp"
#include "backend/services/QuestionService.hpp"
#include "backend/services/RoadmapService.hpp"
#include "backend/services/TeacherConfigurationService.hpp"
#include "tests/common/TestSupport.hpp"

#include <filesystem>

/**
 * @brief Exercises tool registry behavior.
 * @return Process exit code.
 */
int main() {
  edu_ai::tests::Suite suite("tool_registry");
  using edu_ai::ai::ToolContext;
  using edu_ai::ai::ToolDefinition;
  using edu_ai::ai::ToolExecutionStatus;
  using edu_ai::ai::ToolInvocation;
  using edu_ai::ai::ToolMode;
  using edu_ai::ai::ToolRegistry;
  using edu_ai::ai::ToolResult;
  using edu_ai::common::Result;
  using edu_ai::models::Role;

  ToolRegistry registry;
  bool write_handler_called = false;
  suite.scenario("Load the JSON tool catalog from backend/ai/tool_definitions.json");
  const auto catalog =
      edu_ai::ai::load_tool_definitions(std::filesystem::path(EDU_AI_SOURCE_DIR) / "backend/ai/tool_definitions.json");
  CHECK(suite, catalog.ok() && catalog.value->size() == 14);
  suite.scenario("Register a read-only student tool with a JSON argument schema");
  CHECK(suite, registry.register_tool(
                   {.name = "get_course", .description = "Read an accessible course.", .argument_schema_json = "{}"},
                   [](const ToolContext&, const ToolInvocation&) {
                     return Result<ToolResult>::success({.data_json = "{}"});
                   }));
  suite.scenario("Execute a read-only tool using the authenticated actor context");
  const auto read_result = registry.execute({.actor_id = 7, .role = Role::Student},
                                            {.call_id = "call-read", .name = "get_course", .arguments_json = "{}"});
  CHECK(suite, read_result.ok() && read_result.value->status == ToolExecutionStatus::Success &&
                   read_result.value->call_id == "call-read");

  suite.scenario("Register a teacher-only write tool requiring confirmation");
  CHECK(suite, registry.register_tool({.name = "save_teacher_policy",
                                       .description = "Save course policy.",
                                       .argument_schema_json = "{}",
                                       .minimum_role = Role::Teacher,
                                       .mode = ToolMode::Write,
                                       .confirmation_summary = "Save the teacher policy."},
                                      [&write_handler_called](const ToolContext&, const ToolInvocation&) {
                                        write_handler_called = true;
                                        return Result<ToolResult>::success({.data_json = "{}"});
                                      }));
  suite.scenario("Reject a student invoking a teacher-only tool");
  CHECK(suite, !registry
                    .execute({.actor_id = 7, .role = Role::Student},
                             {.call_id = "call-denied", .name = "save_teacher_policy", .arguments_json = "{}"})
                    .ok());
  suite.scenario("Request confirmation before executing a write tool");
  const auto pending_result =
      registry.execute({.actor_id = 1, .role = Role::Teacher},
                       {.call_id = "call-write", .name = "save_teacher_policy", .arguments_json = "{}"});
  CHECK(suite, pending_result.ok() && pending_result.value->status == ToolExecutionStatus::NeedsConfirmation &&
                   !write_handler_called);
  suite.scenario("Execute a confirmed write tool");
  const auto write_result =
      registry.execute({.actor_id = 1, .role = Role::Teacher, .write_confirmed = true},
                       {.call_id = "call-write", .name = "save_teacher_policy", .arguments_json = "{}"});
  CHECK(suite, write_result.ok() && write_result.value->status == ToolExecutionStatus::Success && write_handler_called);

  suite.scenario("Load seed data from the repository test database before exercising tool-backed reads and writes");
  const auto test_db_path =
      std::filesystem::temp_directory_path() /
      ("edu_ai_tool_registry_seed_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".db");
  edu_ai::repositories::SqliteDatabase database(test_db_path);
  database.initialize_schema(std::filesystem::path(EDU_AI_SOURCE_DIR) / "database/init.sql");
  database.initialize_schema(std::filesystem::path(EDU_AI_SOURCE_DIR) / "database/test_db.sql");

  edu_ai::repositories::SqliteSubjectRepository subjects(database);
  edu_ai::repositories::SqliteUserRepository users(database);
  edu_ai::repositories::SqliteCourseRepository courses(database);
  edu_ai::repositories::SqliteEnrollmentRepository enrollments(database);
  edu_ai::repositories::SqliteTeacherConfigurationRepository teacher_configurations(database);
  edu_ai::repositories::SqliteQuestionRepository questions(database);
  edu_ai::repositories::SqliteRoadmapRepository roadmaps(database);
  edu_ai::repositories::SqliteProgressRepository progress(database);
  edu_ai::repositories::SqliteAssessmentRepository assessments(database);
  edu_ai::repositories::SqliteDocumentChunkRepository document_chunks(database);
  edu_ai::repositories::SqliteLearningSessionRepository learning_sessions(database);
  edu_ai::repositories::SqliteMessageRepository messages(database);

  const auto teacher = users.find_by_email("minh.tran@eduai.vn");
  const auto student = users.find_by_email("nam.student@eduai.vn");
  const auto seeded_course = courses.find_by_id(1);
  const auto seeded_policy = teacher_configurations.find_for_teacher_course(1, 1);
  CHECK(suite, teacher && student && seeded_course && seeded_policy);
  CHECK(suite, teacher->role == Role::Teacher);
  CHECK(suite, student->role == Role::Student);
  CHECK(suite, seeded_course->name == "Course 01");
  CHECK(suite, seeded_policy->policy_text == "Seed teacher policy for course 1");

  edu_ai::services::DefaultCourseService course_service(courses, enrollments, users, document_chunks);
  edu_ai::services::DefaultRoadmapService roadmap_service(roadmaps, courses, enrollments, users);
  edu_ai::services::DefaultProgressService progress_service(progress, courses, enrollments, users);
  edu_ai::services::DefaultQuestionService question_service(questions, courses, enrollments, users);
  edu_ai::services::DefaultAssessmentService assessment_service(assessments, courses, enrollments, users);
  edu_ai::services::DefaultTeacherConfigurationService teacher_configuration_service(teacher_configurations, courses,
                                                                                 users);

  edu_ai::ai::ToolRegistry real_registry;
  const auto registered = edu_ai::ai::register_available_tools(
      real_registry,
      {.courses = course_service,
       .roadmaps = roadmap_service,
       .progress = progress_service,
       .questions = question_service,
       .assessments = assessment_service,
       .teacher_configurations = teacher_configuration_service},
      std::filesystem::path(EDU_AI_SOURCE_DIR) / "backend/ai/tool_definitions.json");
  CHECK(suite, registered.ok() && *registered.value > 0);

  const auto get_course_result = real_registry.execute(
      {.actor_id = student->id, .role = Role::Student},
      {.call_id = "seed-get-course", .name = "get_course", .arguments_json = "{\"course_id\":1}"});
  CHECK(suite, get_course_result.ok() &&
                   get_course_result.value->data_json == "{\"id\":1,\"name\":\"Course 01\"}");

  const auto get_roadmap_result = real_registry.execute(
      {.actor_id = student->id, .role = Role::Student},
      {.call_id = "seed-get-roadmap",
       .name = "get_my_roadmap",
       .arguments_json = "{\"course_id\":1}"});
  CHECK(suite, get_roadmap_result.ok() &&
                   get_roadmap_result.value->data_json == "{\"id\":1,\"title\":\"Seed roadmap 1\"}");

  const auto get_progress_result = real_registry.execute(
      {.actor_id = student->id, .role = Role::Student},
      {.call_id = "seed-get-progress",
       .name = "get_my_progress",
       .arguments_json = "{\"course_id\":1}"});
  CHECK(suite, get_progress_result.ok() && get_progress_result.value->data_json == "{\"count\":1}");

  const auto get_student_progress_result = real_registry.execute(
      {.actor_id = teacher->id, .role = Role::Teacher},
      {.call_id = "seed-student-progress",
       .name = "get_student_progress",
       .arguments_json = "{\"student_id\":6,\"course_id\":1}"});
  CHECK(suite, get_student_progress_result.ok() && get_student_progress_result.value->data_json == "{\"count\":1}");

  const auto get_course_assessments_result = real_registry.execute(
      {.actor_id = teacher->id, .role = Role::Teacher},
      {.call_id = "seed-course-assessments",
       .name = "get_course_assessments",
       .arguments_json = "{\"course_id\":1}"});
  CHECK(suite, get_course_assessments_result.ok() && get_course_assessments_result.value->data_json == "{\"count\":1}");

  const auto write_progress = real_registry.execute(
      {.actor_id = student->id, .role = Role::Student, .write_confirmed = true},
      {.call_id = "seed-progress-update",
       .name = "update_my_progress",
       .arguments_json = "{\"course_id\":1,\"topic\":\"Topic 1\",\"status\":\"LEARNING\",\"progress_value\":0.75}"});
  CHECK(suite, write_progress.ok());
  const auto updated_progress = progress.list(student->id, 1);
  CHECK(suite, !updated_progress.empty() && updated_progress.front().topic == "Topic 1" &&
                   updated_progress.front().progress_value == 0.75);

  const auto write_policy = real_registry.execute(
      {.actor_id = teacher->id, .role = Role::Teacher, .write_confirmed = true},
      {.call_id = "seed-policy-update",
       .name = "save_teacher_policy",
       .arguments_json = "{\"course_id\":1,\"policy_text\":\"Tool updated policy for course 1\"}"});
  CHECK(suite, write_policy.ok());
  const auto policy_after_update = teacher_configurations.find_for_teacher_course(1, 1);
  CHECK(suite, policy_after_update && policy_after_update->policy_text == "Tool updated policy for course 1");

  const auto teacher_policy_read = real_registry.execute(
      {.actor_id = teacher->id, .role = Role::Teacher},
      {.call_id = "seed-read-policy",
       .name = "get_teacher_policy",
       .arguments_json = "{\"course_id\":1}"});
  CHECK(suite, teacher_policy_read.ok() && teacher_policy_read.value->data_json == "{\"id\":1}");

  const auto add_question_result = real_registry.execute(
      {.actor_id = teacher->id, .role = Role::Teacher, .write_confirmed = true},
      {.call_id = "seed-add-question",
       .name = "add_question",
       .arguments_json = "{\"course_id\":1,\"question\":\"Why does TCP matter?\",\"answer\":\"It keeps data delivery reliable.\",\"hint\":\"Think about packet delivery\",\"difficulty\":\"medium\",\"question_type\":\"short_answer\",\"source\":\"teacher\"}"});
  CHECK(suite, add_question_result.ok());
  const auto added_question = questions.find_by_content(1, "matter");
  CHECK(suite, !added_question.empty() && added_question.front().answer == "It keeps data delivery reliable.");

  const auto get_question_result = real_registry.execute(
      {.actor_id = teacher->id, .role = Role::Teacher},
      {.call_id = "seed-get-question",
       .name = "get_question",
       .arguments_json = "{\"question_id\":" + std::to_string(added_question.front().id) + "}"});
  CHECK(suite, get_question_result.ok() && get_question_result.value->data_json.find("\"question\":\"Why does TCP matter?\"") != std::string::npos);

  const auto session = learning_sessions.find_by_id(1);
  const auto message = messages.find_by_content(1, "Seed message content 1");
  CHECK(suite, session && !message.empty());

  const auto add_assessment_result = real_registry.execute(
      {.actor_id = teacher->id, .role = Role::Teacher, .write_confirmed = true},
      {.call_id = "seed-add-assessment",
       .name = "add_thinking_assessment",
       .arguments_json = "{\"student_id\":6,\"course_id\":1,\"message_id\":" +
                         std::to_string(message.front().id) +
                         ",\"score\":0.85,\"dimensions_json\":\"{\\\"reasoning\\\":1}\",\"reasoning\":\"Strong explanation\",\"model_name\":\"tool-model\"}"});
  CHECK(suite, add_assessment_result.ok());
  const auto created_assessment = assessments.list_for_user_course(student->id, 1);
  CHECK(suite, !created_assessment.empty());

  const auto my_assessments_result = real_registry.execute(
      {.actor_id = student->id, .role = Role::Student},
      {.call_id = "seed-my-assessments",
       .name = "get_my_thinking_assessments",
       .arguments_json = "{\"course_id\":1}"});
  CHECK(suite, my_assessments_result.ok() && my_assessments_result.value->data_json.find("\"count\":") != std::string::npos);

  const auto review_assessment_result = real_registry.execute(
      {.actor_id = teacher->id, .role = Role::Teacher, .write_confirmed = true},
      {.call_id = "seed-review-assessment",
       .name = "review_thinking_assessment",
       .arguments_json = "{\"assessment_id\":" + std::to_string(created_assessment.front().id) +
                         ",\"teacher_score\":0.9,\"teacher_feedback\":\"Good reasoning\",\"review_status\":\"reviewed\"}"});
  CHECK(suite, review_assessment_result.ok());
  const auto reviewed_assessment = assessments.find_by_id(created_assessment.front().id);
  CHECK(suite, reviewed_assessment && reviewed_assessment->review_status == "reviewed" &&
                   reviewed_assessment->teacher_score && *reviewed_assessment->teacher_score == 0.9);

  const auto filtered_document_chunks = document_chunks.find_in_documents(1, {1, 2}, "topic");
  CHECK(suite, filtered_document_chunks.size() == 1 && filtered_document_chunks.front().document_id == 1 &&
                   filtered_document_chunks.front().content.find("topic") != std::string::npos);

  const auto find_info_in_courses_result = real_registry.execute(
      {.actor_id = student->id, .role = Role::Student},
      {.call_id = "seed-find-info-in-courses",
       .name = "find_info_in_courses",
       .arguments_json = "{\"course_id\":1,\"document_ids\":[1,2],\"query\":\"topic\"}"});
  CHECK(suite, find_info_in_courses_result.ok() &&
                   find_info_in_courses_result.value->data_json == "{\"count\":1}");

  return suite.finish();
}
