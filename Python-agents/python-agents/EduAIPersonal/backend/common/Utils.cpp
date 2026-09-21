#include "backend/common/Utils.hpp"

#include <sstream>

namespace edu_ai::common {
/**
 * @brief Escapes text for the small JSON response payloads emitted by tools.
 * @param value Text to escape.
 * @return JSON-safe string text.
 */
std::string escape_json(const std::string& value) {
  std::string escaped;
  for (const char character : value) {
    if (character == '"' || character == '\\')
      escaped += '\\';
    escaped += character;
  }
  return escaped;
}

Result<Json::Value> json_object(const std::string& json) {
  Json::Value object;
  Json::CharReaderBuilder builder;
  std::string errors;
  std::istringstream input(json);
  if (!Json::parseFromStream(builder, input, &object, &errors) || !object.isObject())
    return Result<Json::Value>::failure(ErrorCode::ToolArgumentError, "Tool arguments must be a JSON object.");
  return Result<Json::Value>::success(std::move(object));
}

Result<models::Id> json_id(const std::string& json, const std::string& key) {
  const auto object = json_object(json);
  if (!object.ok())
    return Result<models::Id>::failure(object.error->code, object.error->message);
  if (!object.value->isMember(key) || !(*object.value)[key].isInt64())
    return Result<models::Id>::failure(ErrorCode::ToolArgumentError, "Missing integer argument: " + key);
  return Result<models::Id>::success((*object.value)[key].asInt64());
}

Result<std::vector<models::Id>> json_ids(const std::string& json, const std::string& key) {
  const auto object = json_object(json);
  if (!object.ok())
    return Result<std::vector<models::Id>>::failure(object.error->code, object.error->message);
  if (!object.value->isMember(key) || !(*object.value)[key].isArray())
    return Result<std::vector<models::Id>>::failure(ErrorCode::ToolArgumentError,
                                                   "Missing integer-array argument: " + key);

  std::vector<models::Id> values;
  values.reserve((*object.value)[key].size());
  for (const auto& item : (*object.value)[key]) {
    if (!item.isInt64())
      return Result<std::vector<models::Id>>::failure(ErrorCode::ToolArgumentError,
                                                     "All document IDs must be integers.");
    values.push_back(item.asInt64());
  }
  return Result<std::vector<models::Id>>::success(std::move(values));
}

Result<std::string> json_string(const std::string& json, const std::string& key) {
  const auto object = json_object(json);
  if (!object.ok())
    return Result<std::string>::failure(object.error->code, object.error->message);
  if (!object.value->isMember(key) || !(*object.value)[key].isString())
    return Result<std::string>::failure(ErrorCode::ToolArgumentError, "Missing string argument: " + key);
  return Result<std::string>::success((*object.value)[key].asString());
}

Result<std::optional<std::string>> json_optional_string(const std::string& json, const std::string& key) {
  const auto object = json_object(json);
  if (!object.ok())
    return Result<std::optional<std::string>>::failure(object.error->code, object.error->message);
  if (!object.value->isMember(key))
    return Result<std::optional<std::string>>::success(std::nullopt);
  if (!(*object.value)[key].isString())
    return Result<std::optional<std::string>>::failure(ErrorCode::ToolArgumentError,
                                                        "Invalid string argument: " + key);
  return Result<std::optional<std::string>>::success((*object.value)[key].asString());
}

Result<double> json_number(const std::string& json, const std::string& key) {
  const auto object = json_object(json);
  if (!object.ok())
    return Result<double>::failure(object.error->code, object.error->message);
  if (!object.value->isMember(key) || !(*object.value)[key].isNumeric())
    return Result<double>::failure(ErrorCode::ToolArgumentError, "Missing numeric argument: " + key);
  return Result<double>::success((*object.value)[key].asDouble());
}

Json::Value to_json(const models::User& user) {
  Json::Value value(Json::objectValue);
  value["id"] = static_cast<Json::Int64>(user.id);
  value["name"] = user.name;
  value["email"] = user.email;
  value["role"] = user.role == models::Role::Teacher ? "teacher"
                  : user.role == models::Role::Admin ? "admin" : "student";
  value["status"] = user.status;
  return value;
}

Json::Value to_json(const models::Course& course) {
  Json::Value value(Json::objectValue);
  value["id"] = static_cast<Json::Int64>(course.id);
  value["subject_id"] = static_cast<Json::Int64>(course.subject_id);
  value["name"] = course.name;
  value["description"] = course.description;
  value["teacher_id"] = static_cast<Json::Int64>(course.teacher_id);
  value["answer_policy"] = course.answer_policy == models::AnswerPolicy::FullAnswer ? "FULL_ANSWER" :
                           course.answer_policy == models::AnswerPolicy::HintOnly ? "HINT_ONLY" : "GUIDED";
  return value;
}

Json::Value to_json(const models::Question& question) {
  Json::Value value(Json::objectValue);
  value["id"] = static_cast<Json::Int64>(question.id);
  value["course_id"] = static_cast<Json::Int64>(question.course_id);
  value["created_by"] = static_cast<Json::Int64>(question.created_by);
  value["question"] = question.question;
  value["answer"] = question.answer;
  value["hint"] = question.hint;
  value["difficulty"] = question.difficulty;
  value["question_type"] = question.question_type;
  value["source"] = question.source;
  value["status"] = question.status == models::QuestionStatus::Draft ? "draft"
                    : question.status == models::QuestionStatus::Review ? "review"
                    : question.status == models::QuestionStatus::Approved ? "approved"
                                                                          : "archived";
  return value;
}

Json::Value to_json(const models::Document& document) {
  Json::Value value(Json::objectValue);
  value["id"] = static_cast<Json::Int64>(document.id);
  value["course_id"] = static_cast<Json::Int64>(document.course_id);
  value["title"] = document.title;
  value["source_path"] = document.source_path;
  return value;
}

Json::Value to_json(const models::DocumentChunk& chunk) {
  Json::Value value(Json::objectValue);
  value["id"] = static_cast<Json::Int64>(chunk.id);
  value["document_id"] = static_cast<Json::Int64>(chunk.document_id);
  value["chunk_index"] = chunk.chunk_index;
  value["content"] = chunk.content;
  value["embedding"] = chunk.embedding;
  return value;
}

Json::Value to_json(const models::Roadmap& roadmap) {
  Json::Value value(Json::objectValue);
  value["id"] = static_cast<Json::Int64>(roadmap.id);
  value["user_id"] = static_cast<Json::Int64>(roadmap.user_id);
  value["course_id"] = static_cast<Json::Int64>(roadmap.course_id);
  value["title"] = roadmap.title;
  value["content_json"] = roadmap.content_json;
  return value;
}

Json::Value to_json(const models::LearningProgress& progress) {
  Json::Value value(Json::objectValue);
  value["id"] = static_cast<Json::Int64>(progress.id);
  value["user_id"] = static_cast<Json::Int64>(progress.user_id);
  value["course_id"] = static_cast<Json::Int64>(progress.course_id);
  value["topic"] = progress.topic;
  value["status"] = progress.status == models::ProgressStatus::NotStarted ? "NOT_STARTED"
                    : progress.status == models::ProgressStatus::Learning ? "LEARNING"
                    : progress.status == models::ProgressStatus::Completed ? "COMPLETED"
                                                                             : "NEEDS_REVIEW";
  value["progress_value"] = progress.progress_value;
  return value;
}

Json::Value to_json(const models::ThinkingAssessment& assessment) {
  Json::Value value(Json::objectValue);
  value["id"] = static_cast<Json::Int64>(assessment.id);
  value["user_id"] = static_cast<Json::Int64>(assessment.user_id);
  value["course_id"] = static_cast<Json::Int64>(assessment.course_id);
  value["message_id"] = static_cast<Json::Int64>(assessment.message_id);
  value["score"] = assessment.score;
  value["dimensions_json"] = assessment.dimensions_json;
  value["reasoning"] = assessment.reasoning;
  value["model_name"] = assessment.model_name;
  value["review_status"] = assessment.review_status;
  if (assessment.teacher_score.has_value()) value["teacher_score"] = *assessment.teacher_score;
  if (assessment.teacher_feedback.has_value()) value["teacher_feedback"] = *assessment.teacher_feedback;
  return value;
}

Json::Value error_json(const Error& error) {
  Json::Value value(Json::objectValue);
  value["error"] = Json::Value(Json::objectValue);
  value["error"]["code"] = static_cast<int>(error.code);
  value["error"]["message"] = error.message;
  return value;
}

Json::Value success_json(const Json::Value& payload) {
  Json::Value value(Json::objectValue);
  value["success"] = true;
  value["data"] = payload;
  return value;
}

}  // namespace edu_ai::common
