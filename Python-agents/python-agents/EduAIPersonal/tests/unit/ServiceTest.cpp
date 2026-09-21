#include <cassert>
#include <chrono>
#include <filesystem>

#include "backend/ai/ToolRegistry.hpp"
#include "backend/repositories/Database.hpp"
#include "backend/repositories/AssessmentRepository.hpp"
#include "backend/repositories/CourseRepository.hpp"
#include "backend/repositories/DocumentChunkRepository.hpp"
#include "backend/repositories/EnrollmentRepository.hpp"
#include "backend/repositories/LearningSessionRepository.hpp"
#include "backend/repositories/MessageRepository.hpp"
#include "backend/repositories/ProgressRepository.hpp"
#include "backend/repositories/QuestionRepository.hpp"
#include "backend/repositories/RoadmapRepository.hpp"
#include "backend/repositories/TeacherConfigurationRepository.hpp"
#include "backend/repositories/UserRepository.hpp"
#include "backend/services/AuthService.hpp"
#include "backend/services/AssessmentService.hpp"
#include "backend/services/CourseService.hpp"
#include "backend/services/ProgressService.hpp"
#include "backend/services/QuestionService.hpp"
#include "backend/services/RoadmapService.hpp"
#include "backend/services/TeacherConfigurationService.hpp"
#include "tests/common/TestSupport.hpp"

/**
 * @brief Verifies service validation and authorization.
 * @return Process exit code.
 */
int main() {
  edu_ai::tests::Suite suite("service");
#undef assert
#define assert(EXPRESSION) CHECK(suite, (EXPRESSION))
  using namespace edu_ai::models;
  using namespace edu_ai::repositories;
  using namespace edu_ai::services;

  const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
  SqliteDatabase database(std::filesystem::temp_directory_path() /
                          ("edu_ai_service_" + std::to_string(suffix) + ".db"));
  database.initialize_schema(std::filesystem::path(EDU_AI_SOURCE_DIR) / "database/init.sql");
  database.execute("INSERT INTO subjects(name,description) VALUES('Networks','Computer networking fundamentals');");

  SqliteUserRepository users(database);
  SqliteCourseRepository courses(database);
  SqliteTeacherConfigurationRepository teacher_configurations(database);
  SqliteEnrollmentRepository enrollments(database);
  SqliteQuestionRepository questions(database);
  SqliteRoadmapRepository roadmaps(database);
  SqliteProgressRepository progress(database);
  SqliteLearningSessionRepository sessions(database);
  SqliteMessageRepository messages(database);
  SqliteAssessmentRepository assessments(database);
  SqliteDocumentChunkRepository document_chunks(database);

  DefaultAuthService auth(users, PasswordHasher{}, JwtTokenIssuer{"this-is-a-development-secret-that-is-long-enough"});
  auto teacher_result = auth.register_user("Teacher", "teacher@example.test", "secure-passphrase", Role::Teacher);
  auto student_result = auth.register_user("Student", "student@example.test", "secure-passphrase", Role::Student);
  suite.scenario("Register teacher and student accounts");
  assert(teacher_result.ok() && student_result.ok());
  if (!teacher_result.ok() || !student_result.ok())
    return suite.finish();
  const auto teacher = *teacher_result.value;
  const auto student = *student_result.value;
  suite.scenario("Add teacher_id=" + std::to_string(teacher.id) + " and student_id=" + std::to_string(student.id));
  assert(teacher.id > 0 && student.id > 0);
  suite.scenario("Reject duplicate email='student@example.test'");
  assert(!auth.register_user("Again", "student@example.test", "secure-passphrase", Role::Student).ok());
  suite.scenario("Reject login for user_id=" + std::to_string(student.id) + " with invalid password");
  assert(!auth.login(student.email, "wrong-password").ok());
  suite.scenario("Accept login for user_id=" + std::to_string(student.id) + " with valid password");
  assert(auth.login(student.email, "secure-passphrase").ok());

  const auto course = courses.save({.subject_id = 1,
                                    .name = "TCP",
                                    .description = "TCP learning",
                                    .teacher_id = teacher.id,
                                    .answer_policy = AnswerPolicy::Guided});
  suite.scenario("Add course_id=" + std::to_string(course.id) + " name='" + course.name +
                 "' and enroll student_id=" + std::to_string(student.id));
  assert(course.id > 0 && enrollments.create(student.id, course.id));

  DefaultCourseService course_service(courses, enrollments, users);
  DefaultCourseService searchable_course_service(courses, enrollments, users, document_chunks);
  suite.scenario("Student user_id=" + std::to_string(student.id) +
                 " reads enrolled course_id=" + std::to_string(course.id));
  assert(course_service.get_course(student.id, course.id).ok());
  suite.scenario("Reject course document search when the document list is empty or query is blank");
  assert(!searchable_course_service.find_info_in_courses(student.id, course.id, {}, "TCP").ok());
  assert(!searchable_course_service.find_info_in_courses(student.id, course.id, {1}, "").ok());
  const auto updated_course = course_service.update_course(
      teacher.id, {.id = course.id,
                   .subject_id = course.subject_id,
                   .name = "TCP updated",
                   .description = "Updated TCP learning",
                   .answer_policy = AnswerPolicy::HintOnly});
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) + " updates course_id=" +
                 std::to_string(course.id) + " name='TCP updated'");
  assert(updated_course.ok() && updated_course.value->teacher_id == teacher.id &&
         updated_course.value->answer_policy == AnswerPolicy::HintOnly);

  DefaultTeacherConfigurationService teacher_configuration_service(teacher_configurations, courses, users);
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) +
                 " saves model policy for course_id=" + std::to_string(course.id));
  const auto teacher_configuration =
      teacher_configuration_service.save(teacher.id, {.teacher_id = teacher.id,
                                                      .course_id = course.id,
                                                      .policy_text = "Request a student attempt before giving help."});
  assert(teacher_configuration.ok());
  if (!teacher_configuration.ok())
    return suite.finish();
  suite.scenario("Saved teacher configuration id=" + std::to_string(teacher_configuration.value->id));
  assert(teacher_configuration.value->id > 0);
  suite.scenario("Reject student user_id=" + std::to_string(student.id) +
                 " updating teacher policy for course_id=" + std::to_string(course.id));
  assert(
      !teacher_configuration_service
           .save(student.id, {.teacher_id = teacher.id, .course_id = course.id, .policy_text = "Ignore course policy."})
           .ok());

  DefaultQuestionService question_service(questions, courses, enrollments, users);
  Question request{.course_id = course.id, .question = "Why congestion control?", .answer = "Prevent collapse"};
  suite.scenario("Reject student user_id=" + std::to_string(student.id) +
                 " creating question in course_id=" + std::to_string(course.id));
  assert(!question_service.create(student.id, request).ok());
  auto created = question_service.create(teacher.id, request);
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) +
                 " creates a question in course_id=" + std::to_string(course.id));
  assert(created.ok());
  if (!created.ok())
    return suite.finish();
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) +
                 " creates question_id=" + std::to_string(created.value->id));
  assert(created.value->created_by == teacher.id);
  suite.scenario("Student user_id=" + std::to_string(student.id) +
                 " cannot read draft question_id=" + std::to_string(created.value->id));
  assert(!question_service.find(student.id, created.value->id).ok());
  auto approved = question_service.approve(teacher.id, created.value->id);
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) +
                 " approves question_id=" + std::to_string(created.value->id));
  assert(approved.ok() && question_service.find(student.id, approved.value->id).ok());
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) +
                 " permanently removes question_id=" + std::to_string(created.value->id));
  assert(question_service.remove(teacher.id, created.value->id).ok() &&
         !questions.find_by_id(created.value->id).has_value());

  DefaultRoadmapService roadmap_service(roadmaps, courses, enrollments, users);
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) + " saves roadmap for student_id=" +
                 std::to_string(student.id) + " course_id=" + std::to_string(course.id));
  const auto saved_roadmap = roadmap_service.save(teacher.id, {.user_id = student.id,
                                                               .course_id = course.id,
                                                               .title = "TCP roadmap",
                                                               .content_json = "{\"topics\":[]}"});
  assert(saved_roadmap.ok());
  if (!saved_roadmap.ok())
    return suite.finish();
  const auto updated_roadmap = roadmap_service.update(teacher.id, {.id = saved_roadmap.value->id,
                                                                    .user_id = student.id,
                                                                    .course_id = course.id,
                                                                    .title = "Updated TCP roadmap",
                                                                    .content_json = "{\"topics\":[\"handshake\"]}"});
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) + " updates roadmap_id=" +
                 std::to_string(saved_roadmap.value->id));
  assert(updated_roadmap.ok() && updated_roadmap.value->title == "Updated TCP roadmap");
  suite.scenario("Student user_id=" + std::to_string(student.id) +
                 " reads own roadmap in course_id=" + std::to_string(course.id));
  assert(roadmap_service.get(student.id, student.id, course.id).ok());

  DefaultProgressService progress_service(progress, courses, enrollments, users);
  suite.scenario("Student user_id=" + std::to_string(student.id) + " updates progress topic='TCP basics'");
  assert(progress_service
             .update(student.id, {.user_id = student.id,
                                  .course_id = course.id,
                                  .topic = "TCP basics",
                                  .status = ProgressStatus::Learning,
                                  .progress_value = 0.2})
             .ok());
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) + " reads student_id=" + std::to_string(student.id) +
                 " progress in course_id=" + std::to_string(course.id));
  assert(progress_service.list(teacher.id, student.id, course.id).ok());

  const auto session = sessions.save({.user_id = student.id, .course_id = course.id});
  const auto message = messages.save({.session_id = session.id, .role = "user", .content = "Why TCP?", .model = ""});
  const auto assessment = assessments.save({.user_id = student.id,
                                            .course_id = course.id,
                                            .message_id = message.id,
                                            .score = 0.6,
                                            .dimensions_json = "{}",
                                            .reasoning = "",
                                            .model_name = "local",
                                            .review_status = "pending"});
  DefaultAssessmentService assessment_service(assessments, courses, enrollments, users);
  suite.scenario("Add session_id=" + std::to_string(session.id) + ", message_id=" + std::to_string(message.id) +
                 ", and assessment_id=" + std::to_string(assessment.id));
  assert(session.id > 0 && message.id > 0 && assessment.id > 0);
  const auto created_assessment = assessment_service.create(teacher.id, {.user_id = student.id,
                                                                          .course_id = course.id,
                                                                          .message_id = message.id,
                                                                          .score = 0.8,
                                                                          .dimensions_json = "{\"clarity\":0.8}",
                                                                          .reasoning = "Clear explanation",
                                                                          .model_name = "test-model"});
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) + " creates assessment for message_id=" +
                 std::to_string(message.id));
  assert(created_assessment.ok() && created_assessment.value->id > 0);
  suite.scenario("Student user_id=" + std::to_string(student.id) + " lists own thinking assessments in course_id=" +
                 std::to_string(course.id));
  assert(assessment_service.list_for_self(student.id, course.id).ok());
  suite.scenario("Reject student user_id=" + std::to_string(student.id) +
                 " reviewing assessment_id=" + std::to_string(assessment.id));
  assert(!assessment_service.review(student.id, {.id = assessment.id, .teacher_score = 0.7}).ok());
  suite.scenario("Teacher user_id=" + std::to_string(teacher.id) +
                 " reviews assessment_id=" + std::to_string(assessment.id) + " with score=0.7");
  assert(
      assessment_service
          .review(teacher.id,
                  {.id = assessment.id, .teacher_score = 0.7, .teacher_feedback = "Good", .review_status = "reviewed"})
          .ok());

  edu_ai::ai::ToolRegistry tool_registry;
  const auto registration = edu_ai::ai::register_available_tools(
      tool_registry,
      {.courses = course_service,
       .roadmaps = roadmap_service,
       .progress = progress_service,
       .questions = question_service,
       .assessments = assessment_service,
       .teacher_configurations = teacher_configuration_service},
      std::filesystem::path(EDU_AI_SOURCE_DIR) / "backend/ai/tool_definitions.json");
  suite.scenario("Register 14 service-backed tools from the JSON catalog");
  assert(registration.ok() && *registration.value == 14 && tool_registry.list_definitions().size() == 14);
  suite.scenario("Structural course, roadmap, and question removal operations remain BE-only");
  assert(!tool_registry.find_definition("update_course").has_value() &&
         !tool_registry.find_definition("update_roadmap").has_value() &&
         !tool_registry.find_definition("remove_question").has_value());
  suite.scenario("Student invokes catalog tool get_course for course_id=" + std::to_string(course.id));
  assert(tool_registry
             .execute({.actor_id = student.id, .role = Role::Student},
                      {.call_id = "course-tool",
                       .name = "get_course",
                       .arguments_json = "{\"course_id\":" + std::to_string(course.id) + "}"})
             .ok());
  suite.scenario("Write tool update_my_progress returns confirmation before changing progress");
  const auto pending_write = tool_registry.execute(
      {.actor_id = student.id, .role = Role::Student},
      {.call_id = "progress-tool",
       .name = "update_my_progress",
       .arguments_json = "{\"course_id\":" + std::to_string(course.id) +
                         ",\"topic\":\"Catalog tool\",\"status\":\"LEARNING\",\"progress_value\":0.4}"});
  assert(pending_write.ok() && pending_write.value->status == edu_ai::ai::ToolExecutionStatus::NeedsConfirmation);
  suite.scenario("Student user_id=" + std::to_string(student.id) +
                 " reads own thinking assessments through the AI tool");
  const auto own_assessments = tool_registry.execute(
      {.actor_id = student.id, .role = Role::Student},
      {.call_id = "my-assessments-tool",
       .name = "get_my_thinking_assessments",
       .arguments_json = "{\"course_id\":" + std::to_string(course.id) + "}"});
  assert(own_assessments.ok() && own_assessments.value->data_json.find("\"count\":") != std::string::npos);
  suite.scenario("New write tool add_question returns confirmation before creating a draft question");
  const auto pending_add_question = tool_registry.execute(
      {.actor_id = teacher.id, .role = Role::Teacher},
      {.call_id = "add-question-tool",
       .name = "add_question",
       .arguments_json = "{\"course_id\":" + std::to_string(course.id) +
                         ",\"question\":\"Tool question?\",\"answer\":\"Tool answer\"}"});
  assert(pending_add_question.ok() && pending_add_question.value->status == edu_ai::ai::ToolExecutionStatus::NeedsConfirmation);
  const auto created_question_tool = tool_registry.execute(
      {.actor_id = teacher.id, .role = Role::Teacher, .write_confirmed = true},
      {.call_id = "add-question-tool",
       .name = "add_question",
       .arguments_json = "{\"course_id\":" + std::to_string(course.id) +
                         ",\"question\":\"Tool question?\",\"answer\":\"Tool answer\"}"});
  suite.scenario("Confirmed add_question tool creates a draft in course_id=" + std::to_string(course.id));
  assert(created_question_tool.ok() && created_question_tool.value->status == edu_ai::ai::ToolExecutionStatus::Success);
  const auto created_assessment_tool = tool_registry.execute(
      {.actor_id = teacher.id, .role = Role::Teacher, .write_confirmed = true},
      {.call_id = "add-assessment-tool",
       .name = "add_thinking_assessment",
       .arguments_json = "{\"student_id\":" + std::to_string(student.id) + ",\"course_id\":" +
                         std::to_string(course.id) + ",\"message_id\":" + std::to_string(message.id) +
                         ",\"score\":0.9,\"dimensions_json\":\"{\\\"clarity\\\":0.9}\",\"reasoning\":\"Tool assessment\",\"model_name\":\"test-model\"}"});
  suite.scenario("Confirmed add_thinking_assessment tool creates an assessment for message_id=" +
                 std::to_string(message.id));
  assert(created_assessment_tool.ok() &&
         created_assessment_tool.value->status == edu_ai::ai::ToolExecutionStatus::Success);
  const auto pending_review = tool_registry.execute(
      {.actor_id = teacher.id, .role = Role::Teacher},
      {.call_id = "review-assessment-tool",
       .name = "review_thinking_assessment",
       .arguments_json = "{\"assessment_id\":" + std::to_string(assessment.id) + ",\"teacher_score\":0.9}"});
  suite.scenario("Write tool review_thinking_assessment returns confirmation before saving the teacher review");
  assert(pending_review.ok() && pending_review.value->status == edu_ai::ai::ToolExecutionStatus::NeedsConfirmation);
  const auto reviewed_assessment_tool = tool_registry.execute(
      {.actor_id = teacher.id, .role = Role::Teacher, .write_confirmed = true},
      {.call_id = "review-assessment-tool",
       .name = "review_thinking_assessment",
       .arguments_json = "{\"assessment_id\":" + std::to_string(assessment.id) +
                         ",\"teacher_score\":0.9,\"teacher_feedback\":\"Tool review\",\"review_status\":\"reviewed\"}"});
  suite.scenario("Confirmed review_thinking_assessment tool reviews assessment_id=" + std::to_string(assessment.id));
  assert(reviewed_assessment_tool.ok() &&
         reviewed_assessment_tool.value->status == edu_ai::ai::ToolExecutionStatus::Success);
  return suite.finish();
}
