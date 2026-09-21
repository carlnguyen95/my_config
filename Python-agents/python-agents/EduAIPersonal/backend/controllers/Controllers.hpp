#pragma once

#include <optional>
#include <string>
#include <vector>

#include <json/json.h>

#ifdef EDU_AI_ENABLE_DROGON
#include <drogon/drogon.h>
#endif

#include "backend/common/Result.hpp"
#include "backend/common/Utils.hpp"
#include "backend/bootstrap/HttpRuntime.hpp"
#include "backend/models/Domain.hpp"
#include "backend/repositories/Repositories.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::controllers {

// HTTP adapters belong here. The controllers stay thin and delegate all business rules to services.
class AuthController
#ifdef EDU_AI_ENABLE_DROGON
    : public drogon::HttpController<AuthController>
#endif
{
 public:
#ifdef EDU_AI_ENABLE_DROGON
  AuthController()
      : AuthController(bootstrap::HttpRuntime::instance().auth_service(), bootstrap::HttpRuntime::instance().users()) {}

  METHOD_LIST_BEGIN
  ADD_METHOD_TO(AuthController::me, "/api/auth/me", drogon::Get, drogon::Options);
  ADD_METHOD_TO(AuthController::list_users, "/api/auth/users", drogon::Get, drogon::Options);
  ADD_METHOD_TO(AuthController::login, "/api/auth/login", drogon::Post, drogon::Options);
  METHOD_LIST_END
#endif

  AuthController(services::AuthService& auth_service, repositories::UserRepository& users)
      : auth_service_(auth_service), users_(users) {}

  Json::Value me_data(models::Id user_id) const {
    const auto user = users_.find_by_id(user_id);
    if (!user.has_value()) {
      return common::error_json(*common::Result<models::User>::failure(common::ErrorCode::NotFound, "User not found.").error);
    }
    Json::Value result(Json::objectValue);
    result["user"] = common::to_json(*user);
    return result;
  }

  Json::Value list_users_data() const {
    Json::Value result(Json::objectValue);
    Json::Value list(Json::arrayValue);
    for (const auto& user : users_.find_by_status("active")) {
      list.append(common::to_json(user));
    }
    result["users"] = list;
    return result;
  }

  Json::Value login_data(const std::string& email, const std::string& password) const {
    const auto result = auth_service_.login(email, password);
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    Json::Value payload(Json::objectValue);
    payload["token"] = result.value.value();
    payload["user"] = common::to_json(*users_.find_by_email(email));
    return payload;
  }

#ifdef EDU_AI_ENABLE_DROGON
  void login(const drogon::HttpRequestPtr& req,
             std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    const auto& json = req->getJsonObject();
    const std::string email = json ? (*json)["email"].asString() : "";
    const std::string password = json ? (*json)["password"].asString() : "";
    callback(drogon::HttpResponse::newHttpJsonResponse(login_data(email, password)));
  }

  void me(const drogon::HttpRequestPtr& req,
          std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    const auto user_id = req->getParameter("user_id");
    callback(drogon::HttpResponse::newHttpJsonResponse(me_data(user_id.empty() ? 0 : std::stoll(user_id))));
  }

  void list_users(const drogon::HttpRequestPtr&,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    callback(drogon::HttpResponse::newHttpJsonResponse(list_users_data()));
  }
#endif

  Json::Value register_user(const std::string& name, const std::string& email, const std::string& password,
                           models::Role role) const {
    const auto result = auth_service_.register_user(name, email, password, role);
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    return common::success_json(common::to_json(*result.value));
  }

 private:
  services::AuthService& auth_service_;
  repositories::UserRepository& users_;
};

class CourseController
#ifdef EDU_AI_ENABLE_DROGON
    : public drogon::HttpController<CourseController>
#endif
{
 public:
#ifdef EDU_AI_ENABLE_DROGON
  CourseController()
      : CourseController(bootstrap::HttpRuntime::instance().course_service(), bootstrap::HttpRuntime::instance().courses(),
                         bootstrap::HttpRuntime::instance().documents(), bootstrap::HttpRuntime::instance().document_chunks(),
                         bootstrap::HttpRuntime::instance().teacher_configurations()) {}

  METHOD_LIST_BEGIN
  ADD_METHOD_TO(CourseController::list_courses, "/api/courses", drogon::Get, drogon::Options);
  ADD_METHOD_TO(CourseController::get_course, "/api/courses/{1}", drogon::Get, drogon::Options);
  ADD_METHOD_TO(CourseController::update_policy, "/api/courses/{1}/policy", drogon::Patch, drogon::Options);
  ADD_METHOD_TO(CourseController::list_documents, "/api/courses/{1}/documents", drogon::Get, drogon::Options);
  ADD_METHOD_TO(CourseController::upload_document, "/api/courses/{1}/documents", drogon::Post, drogon::Options);
  METHOD_LIST_END
#endif

  CourseController(services::CourseService& course_service, repositories::CourseRepository& courses,
                  repositories::DocumentRepository& documents, repositories::DocumentChunkRepository& document_chunks,
                  repositories::TeacherConfigurationRepository& teacher_configurations)
      : course_service_(course_service),
        courses_(courses),
        documents_(documents),
        document_chunks_(document_chunks),
        teacher_configurations_(teacher_configurations) {}

  Json::Value list_courses_data() const {
    Json::Value list(Json::arrayValue);
    for (const auto& course : courses_.find_by_name("")) {
      list.append(common::to_json(course));
    }
    Json::Value result(Json::objectValue);
    result["courses"] = list;
    return result;
  }

  Json::Value get_course_data(models::Id actor_id, models::Id course_id) const {
    const auto result = course_service_.get_course(actor_id, course_id);
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    Json::Value payload(Json::objectValue);
    payload["course"] = common::to_json(*result.value);
    return payload;
  }

#ifdef EDU_AI_ENABLE_DROGON
  void list_courses(const drogon::HttpRequestPtr&,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    callback(drogon::HttpResponse::newHttpJsonResponse(list_courses_data()));
  }

  void get_course(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::string course_id) {
    const auto actor_id = req->getParameter("actor_id");
    callback(drogon::HttpResponse::newHttpJsonResponse(
        get_course_data(actor_id.empty() ? 0 : std::stoll(actor_id), std::stoll(course_id))));
  }

  void update_policy(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::string course_id) {
    const auto json = req->getJsonObject();
    const auto actor_id = req->getParameter("actor_id");
    const auto policy = json ? static_cast<models::AnswerPolicy>((*json)["answer_policy"].asInt())
                            : models::AnswerPolicy::Guided;
    callback(drogon::HttpResponse::newHttpJsonResponse(
        update_policy_data(actor_id.empty() ? 0 : std::stoll(actor_id), std::stoll(course_id), policy)));
  }

  void list_documents(const drogon::HttpRequestPtr&,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::string course_id) {
    callback(drogon::HttpResponse::newHttpJsonResponse(list_documents_data(std::stoll(course_id))));
  }

  void upload_document(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::string course_id) {
    const auto json = req->getJsonObject();
    std::vector<std::string> tags;
    if (json && (*json).isMember("tags") && (*json)["tags"].isArray()) {
      for (const auto& item : (*json)["tags"]) tags.push_back(item.asString());
    }
    const auto actor_id = req->getParameter("actor_id");
    const std::string title = json ? (*json)["title"].asString() : "";
    const std::string content = json ? (*json)["content"].asString() : "";
    callback(drogon::HttpResponse::newHttpJsonResponse(
        upload_document_data(actor_id.empty() ? 0 : std::stoll(actor_id), std::stoll(course_id), title, content, tags)));
  }
#endif

  Json::Value update_policy_data(models::Id actor_id, models::Id course_id, models::AnswerPolicy policy) const {
    auto existing = courses_.find_by_id(course_id);
    if (!existing.has_value()) {
      return common::error_json(*common::Result<models::Course>::failure(common::ErrorCode::NotFound, "Course not found.").error);
    }
    auto updated = *existing;
    updated.answer_policy = policy;
    const auto saved = courses_.save(updated);
    Json::Value payload(Json::objectValue);
    payload["course"] = common::to_json(saved);
    payload["success"] = true;
    return payload;
  }

  Json::Value list_documents_data(models::Id course_id) const {
    Json::Value result(Json::objectValue);
    Json::Value list(Json::arrayValue);
    for (const auto& doc : documents_.find_by_course(course_id)) {
      list.append(common::to_json(doc));
    }
    result["documents"] = list;
    return result;
  }

  Json::Value upload_document_data(models::Id actor_id, models::Id course_id, const std::string& title,
                                  const std::string& content, const std::vector<std::string>& tags) const {
    const auto existing = courses_.find_by_id(course_id);
    if (!existing.has_value()) {
      return common::error_json(*common::Result<models::Document>::failure(common::ErrorCode::NotFound, "Course not found.").error);
    }
    auto document = models::Document{.course_id = course_id,
                                    .title = title.empty() ? "Uploaded document" : title,
                                    .source_path = "/uploads/" + std::to_string(course_id) + ".txt"};
    const auto saved_doc = documents_.save(document);
    const auto chunk = models::DocumentChunk{.document_id = saved_doc.id,
                                             .chunk_index = 0,
                                             .content = content.empty() ? "Uploaded course material." : content,
                                             .embedding = "[]"};
    const auto saved_chunk = document_chunks_.save(chunk);
    Json::Value payload(Json::objectValue);
    payload["success"] = true;
    payload["document"] = common::to_json(saved_doc);
    payload["chunk"] = common::to_json(saved_chunk);
    return payload;
  }

 private:
  services::CourseService& course_service_;
  repositories::CourseRepository& courses_;
  repositories::DocumentRepository& documents_;
  repositories::DocumentChunkRepository& document_chunks_;
  repositories::TeacherConfigurationRepository& teacher_configurations_;
};

class QuestionController {
 public:
  QuestionController(services::QuestionService& question_service, repositories::QuestionRepository& questions)
      : question_service_(question_service), questions_(questions) {}

  Json::Value list_for_course(models::Id course_id, const std::string& role, const std::optional<std::string>& status,
                             const std::optional<std::string>& difficulty,
                             const std::optional<std::string>& query) const {
    Json::Value result(Json::objectValue);
    Json::Value list(Json::arrayValue);
    std::vector<models::Question> items = questions_.find_by_content(course_id, query.value_or(""));
    for (const auto& item : items) {
      if (status.has_value() && item.status != models::QuestionStatus::Draft && status.value() == "draft") {
        continue;
      }
      if (difficulty.has_value() && item.difficulty != difficulty.value()) {
        continue;
      }
      list.append(common::to_json(item));
    }
    result["questions"] = list;
    return result;
  }

  Json::Value create(models::Id actor_id, models::Id course_id, const std::string& question_text,
                     const std::string& answer, const std::string& hint, const std::string& difficulty,
                     const std::string& question_type, const std::string& source) const {
    auto result = question_service_.create(
        actor_id, {.course_id = course_id,
                   .question = question_text,
                   .answer = answer,
                   .hint = hint,
                   .difficulty = difficulty,
                   .question_type = question_type,
                   .source = source});
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    return common::success_json(common::to_json(*result.value));
  }

  Json::Value update(models::Id actor_id, models::Id question_id, const std::optional<std::string>& question_text,
                     const std::optional<std::string>& answer, const std::optional<std::string>& hint,
                     const std::optional<std::string>& difficulty, const std::optional<std::string>& status) const {
    auto found = questions_.find_by_id(question_id);
    if (!found.has_value()) {
      return common::error_json(*common::Result<models::Question>::failure(common::ErrorCode::NotFound, "Question not found.").error);
    }
    auto updated = *found;
    if (question_text.has_value()) updated.question = *question_text;
    if (answer.has_value()) updated.answer = *answer;
    if (hint.has_value()) updated.hint = *hint;
    if (difficulty.has_value()) updated.difficulty = *difficulty;
    if (status.has_value()) {
      if (status == "draft") updated.status = models::QuestionStatus::Draft;
      else if (status == "review") updated.status = models::QuestionStatus::Review;
      else if (status == "approved") updated.status = models::QuestionStatus::Approved;
      else if (status == "archived") updated.status = models::QuestionStatus::Archived;
    }
    const auto saved = questions_.save(updated);
    return common::success_json(common::to_json(saved));
  }

  Json::Value approve(models::Id actor_id, models::Id question_id) const {
    const auto result = question_service_.approve(actor_id, question_id);
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    return common::success_json(common::to_json(*result.value));
  }

 private:
  services::QuestionService& question_service_;
  repositories::QuestionRepository& questions_;
};

class LearningController {
 public:
  LearningController(repositories::MessageRepository& messages, repositories::LearningSessionRepository& sessions)
      : messages_(messages), sessions_(sessions) {}

  Json::Value list_messages(models::Id session_id) const {
    Json::Value result(Json::objectValue);
    Json::Value list(Json::arrayValue);
    for (const auto& message : messages_.recent_for_session(session_id, 50)) {
      Json::Value item(Json::objectValue);
      item["id"] = static_cast<Json::Int64>(message.id);
      item["session_id"] = static_cast<Json::Int64>(message.session_id);
      item["role"] = message.role;
      item["content"] = message.content;
      if (message.token_input.has_value()) item["token_input"] = *message.token_input;
      if (message.token_output.has_value()) item["token_output"] = *message.token_output;
      item["model"] = message.model;
      list.append(item);
    }
    result["messages"] = list;
    return result;
  }

  Json::Value chat(models::Id user_id, models::Id course_id, const std::string& message) const {
    Json::Value result(Json::objectValue);
    result["message_id"] = "chat_message";
    result["answer"] = "Controller adapter placeholder for chat orchestration.";
    result["user_id"] = static_cast<Json::Int64>(user_id);
    result["course_id"] = static_cast<Json::Int64>(course_id);
    result["tool_calls"] = Json::Value(Json::arrayValue);
    result["rag_sources"] = Json::Value(Json::arrayValue);
    return result;
  }

 private:
  repositories::MessageRepository& messages_;
  repositories::LearningSessionRepository& sessions_;
};

class RoadmapController {
 public:
  RoadmapController(services::RoadmapService& roadmap_service, repositories::RoadmapRepository& roadmaps)
      : roadmap_service_(roadmap_service), roadmaps_(roadmaps) {}

  Json::Value get(models::Id actor_id, models::Id user_id, models::Id course_id) const {
    const auto result = roadmap_service_.get(actor_id, user_id, course_id);
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    Json::Value payload(Json::objectValue);
    payload["roadmap"] = common::to_json(*result.value);
    return payload;
  }

  Json::Value save(models::Id actor_id, models::Id user_id, models::Id course_id, const std::string& title,
                  const std::string& content_json) const {
    const auto result = roadmap_service_.save(
        actor_id, {.user_id = user_id, .course_id = course_id, .title = title, .content_json = content_json});
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    return common::success_json(common::to_json(*result.value));
  }

  Json::Value update(models::Id actor_id, models::Id roadmap_id, models::Id user_id, models::Id course_id,
                     const std::string& title, const std::string& content_json) const {
    auto existing = roadmaps_.find_by_id(roadmap_id);
    if (!existing.has_value()) {
      return common::error_json(*common::Result<models::Roadmap>::failure(common::ErrorCode::NotFound, "Roadmap not found.").error);
    }
    auto updated = *existing;
    updated.user_id = user_id;
    updated.course_id = course_id;
    updated.title = title;
    updated.content_json = content_json;
    const auto saved = roadmaps_.save(updated);
    return common::success_json(common::to_json(saved));
  }

 private:
  services::RoadmapService& roadmap_service_;
  repositories::RoadmapRepository& roadmaps_;
};

class ProgressController {
 public:
  ProgressController(services::ProgressService& progress_service)
      : progress_service_(progress_service) {}

  Json::Value list(models::Id actor_id, models::Id user_id, models::Id course_id) const {
    const auto result = progress_service_.list(actor_id, user_id, course_id);
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    Json::Value list(Json::arrayValue);
    for (const auto& item : *result.value) {
      list.append(common::to_json(item));
    }
    Json::Value payload(Json::objectValue);
    payload["progress"] = list;
    return payload;
  }

  Json::Value update(models::Id actor_id, models::Id user_id, models::Id course_id, const std::string& topic,
                    models::ProgressStatus status, double progress_value) const {
    const auto result = progress_service_.update(
        actor_id, {.user_id = user_id, .course_id = course_id, .topic = topic, .status = status, .progress_value = progress_value});
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    return common::success_json(common::to_json(*result.value));
  }

 private:
  services::ProgressService& progress_service_;
};

class AssessmentController {
 public:
  AssessmentController(services::AssessmentService& assessment_service, repositories::AssessmentRepository& assessments)
      : assessment_service_(assessment_service), assessments_(assessments) {}

  Json::Value list(models::Id actor_id, models::Id course_id, const std::optional<models::Id>& user_id,
                   const std::optional<std::string>& review_status) const {
    std::vector<models::ThinkingAssessment> items;
    if (user_id.has_value()) {
      const auto result = assessment_service_.list_for_self(actor_id, course_id);
      if (!result.ok()) return common::error_json(*result.error);
      items = *result.value;
    } else {
      const auto result = assessment_service_.list_for_course(actor_id, course_id);
      if (!result.ok()) return common::error_json(*result.error);
      items = *result.value;
    }
    if (review_status.has_value()) {
      std::vector<models::ThinkingAssessment> filtered;
      for (const auto& item : items) {
        if (item.review_status == *review_status) filtered.push_back(item);
      }
      items = filtered;
    }
    Json::Value list_json(Json::arrayValue);
    for (const auto& item : items) list_json.append(common::to_json(item));
    Json::Value payload(Json::objectValue);
    payload["assessments"] = list_json;
    return payload;
  }

  Json::Value get(models::Id assessment_id) const {
    const auto assessment = assessments_.find_by_id(assessment_id);
    if (!assessment.has_value()) {
      return common::error_json(*common::Result<models::ThinkingAssessment>::failure(common::ErrorCode::NotFound, "Assessment not found.").error);
    }
    Json::Value payload(Json::objectValue);
    payload["assessment"] = common::to_json(*assessment);
    return payload;
  }

  Json::Value review(models::Id actor_id, models::Id assessment_id, double teacher_score,
                    const std::optional<std::string>& teacher_feedback,
                    const std::optional<std::string>& review_status) const {
    const auto result = assessment_service_.review(
        actor_id, {.id = assessment_id,
                   .teacher_score = teacher_score,
                   .teacher_feedback = teacher_feedback.value_or(""),
                   .review_status = review_status.value_or("reviewed")});
    if (!result.ok()) {
      return common::error_json(*result.error);
    }
    return common::success_json(common::to_json(*result.value));
  }

 private:
  services::AssessmentService& assessment_service_;
  repositories::AssessmentRepository& assessments_;
};

}  // namespace edu_ai::controllers
