#include "backend/ai/ToolRegistry.hpp"

#include <fstream>

#include <json/json.h>

#include "backend/common/Utils.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::ai {
using common::escape_json;
using common::json_id;
using common::json_number;
using common::json_optional_string;
using common::json_string;

namespace {

/**
 * @brief Converts progress-status text to the domain enum.
 * @param value Catalog-compatible status text.
 * @return Progress status, defaulting to not started.
 */
models::ProgressStatus progress_status(const std::string& value) {
  return value == "LEARNING"       ? models::ProgressStatus::Learning
         : value == "COMPLETED"    ? models::ProgressStatus::Completed
         : value == "NEEDS_REVIEW" ? models::ProgressStatus::NeedsReview
                                   : models::ProgressStatus::NotStarted;
}

/**
 * @brief Converts a successful service result to a tool result JSON payload.
 * @tparam T Service result value type.
 * @param result Service result to convert.
 * @param data_json Serialized success payload.
 * @return Tool result or propagated service error.
 */
template <typename T>
common::Result<ToolResult> tool_result(const common::Result<T>& result, std::string data_json) {
  if (!result.ok())
    return common::Result<ToolResult>::failure(result.error->code, result.error->message);
  return common::Result<ToolResult>::success({.data_json = std::move(data_json)});
}

/**
 * @brief Checks whether the actual role satisfies a tool's required role.
 * @param actual Role of the authenticated actor.
 * @param required Minimum role configured for the tool.
 * @return True when the actor is authorized.
 */
bool has_role(models::Role actual, models::Role required) {
  return actual == models::Role::Admin || actual == required || required == models::Role::Student;
}
}  // namespace

/**
 * @brief Registers a uniquely named tool and trusted handler.
 * @param definition Tool metadata advertised to an AI provider.
 * @param handler Server-side operation handler.
 * @return True when registration succeeds.
 */
bool ToolRegistry::register_tool(ToolDefinition definition, ToolHandler handler) {
  if (definition.name.empty() || !handler)
    return false;

  const auto tool_name = definition.name;
  return tools_.emplace(tool_name, RegisteredTool{std::move(definition), std::move(handler)}).second;
}

/**
 * @brief Authorizes and executes a tool invocation.
 * @param context Trusted actor identity and confirmation state.
 * @param invocation Tool name, call ID, and arguments.
 * @return Tool result, confirmation requirement, or error.
 */
common::Result<ToolResult> ToolRegistry::execute(const ToolContext& context, const ToolInvocation& invocation) const {
  const auto it = tools_.find(invocation.name);
  if (it == tools_.end())
    return common::Result<ToolResult>::failure(common::ErrorCode::ToolNotFound, "Tool is not registered.");
  if (!has_role(context.role, it->second.definition.minimum_role))
    return common::Result<ToolResult>::failure(common::ErrorCode::ToolPermissionDenied,
                                               "Role is not allowed to invoke this tool.");
  if (it->second.definition.mode == ToolMode::Write && !context.write_confirmed) {
    return common::Result<ToolResult>::success({.call_id = invocation.call_id,
                                                .status = ToolExecutionStatus::NeedsConfirmation,
                                                .confirmation_summary = it->second.definition.confirmation_summary});
  }
  // JSON-schema validation belongs here before handler execution in the implementation phase.
  auto result = it->second.handler(context, invocation);
  if (result.ok() && result.value->call_id.empty())
    result.value->call_id = invocation.call_id;
  return result;
}

/**
 * @brief Finds registered tool metadata by name.
 * @param name Tool name.
 * @return Definition when found; otherwise no value.
 */
std::optional<ToolDefinition> ToolRegistry::find_definition(const std::string& name) const {
  const auto it = tools_.find(name);
  return it == tools_.end() ? std::nullopt : std::optional<ToolDefinition>{it->second.definition};
}

/**
 * @brief Returns every registered tool definition.
 * @return Definitions for provider advertisement.
 */
std::vector<ToolDefinition> ToolRegistry::list_definitions() const {
  std::vector<ToolDefinition> definitions;
  definitions.reserve(tools_.size());
  for (const auto& [name, tool] : tools_) definitions.push_back(tool.definition);
  return definitions;
}

/**
 * @brief Loads and validates tool metadata from the JSON catalog.
 * @param catalog_path Absolute or relative path to tool_definitions.json.
 * @return Catalog definitions, or an error when the file is malformed.
 */
common::Result<std::vector<ToolDefinition>> load_tool_definitions(const std::filesystem::path& catalog_path) {
  std::ifstream input(catalog_path);
  Json::Value root;
  Json::CharReaderBuilder builder;
  std::string errors;
  if (!input || !Json::parseFromStream(builder, input, &root, &errors) || !root["tools"].isArray())
    return common::Result<std::vector<ToolDefinition>>::failure(common::ErrorCode::InvalidRequest,
                                                                "Cannot load tool catalog: " + errors);
  std::vector<ToolDefinition> definitions;
  for (const auto& item : root["tools"]) {
    if (!item["name"].isString() || !item["description"].isString() || !item["parameters"].isObject())
      return common::Result<std::vector<ToolDefinition>>::failure(common::ErrorCode::InvalidRequest,
                                                                  "Tool catalog has an invalid definition.");
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    definitions.push_back(
        {.name = item["name"].asString(),
         .description = item["description"].asString(),
         .argument_schema_json = Json::writeString(writer, item["parameters"]),
         .minimum_role = item["minimum_role"].asString() == "teacher" ? models::Role::Teacher
                         : item["minimum_role"].asString() == "admin" ? models::Role::Admin
                                                                      : models::Role::Student,
         .mode = item["read_only"].asBool() ? ToolMode::Read : ToolMode::Write,
         .confirmation_summary = item.get("confirmation_summary", "Confirm this change.").asString()});
  }
  return common::Result<std::vector<ToolDefinition>>::success(std::move(definitions));
}

/**
 * @brief Registers the production tools backed by the services already implemented.
 * @param registry Registry that owns tool definitions and handlers.
 * @param bindings Service dependencies used by each handler.
 */
common::Result<std::size_t> register_available_tools(ToolRegistry& registry, ServiceToolBindings bindings,
                                                     const std::filesystem::path& catalog_path) {
  const auto catalog = load_tool_definitions(catalog_path);
  if (!catalog.ok())
    return common::Result<std::size_t>::failure(catalog.error->code, catalog.error->message);
  std::unordered_map<std::string, ToolDefinition> definitions;
  for (const auto& definition : *catalog.value) definitions.emplace(definition.name, definition);
  std::unordered_map<std::string, ToolHandler> handlers;
  handlers.emplace(
      "get_course", [&courses = bindings.courses](const ToolContext& context, const ToolInvocation& invocation) {
        const auto course_id = json_id(invocation.arguments_json, "course_id");
        if (!course_id.ok())
          return common::Result<ToolResult>::failure(course_id.error->code, course_id.error->message);
        const auto result = courses.get_course(context.actor_id, *course_id.value);
        return tool_result(result, result.ok() ? "{\"id\":" + std::to_string(result.value->id) + ",\"name\":\"" +
                                                     escape_json(result.value->name) + "\"}"
                                               : "{}");
      });

  handlers.emplace(
      "get_my_roadmap", [&roadmaps = bindings.roadmaps](const ToolContext& context, const ToolInvocation& invocation) {
        const auto course_id = json_id(invocation.arguments_json, "course_id");
        if (!course_id.ok())
          return common::Result<ToolResult>::failure(course_id.error->code, course_id.error->message);
        const auto result = roadmaps.get(context.actor_id, context.actor_id, *course_id.value);
        return tool_result(result, result.ok() ? "{\"id\":" + std::to_string(result.value->id) + ",\"title\":\"" +
                                                     escape_json(result.value->title) + "\"}"
                                               : "{}");
      });

  handlers.emplace(
      "get_my_progress", [&progress = bindings.progress](const ToolContext& context, const ToolInvocation& invocation) {
        const auto course_id = json_id(invocation.arguments_json, "course_id");
        if (!course_id.ok())
          return common::Result<ToolResult>::failure(course_id.error->code, course_id.error->message);
        const auto result = progress.list(context.actor_id, context.actor_id, *course_id.value);
        return tool_result(result, result.ok() ? "{\"count\":" + std::to_string(result.value->size()) + "}" : "{}");
      });

  handlers.emplace("get_student_progress", [&progress = bindings.progress](const ToolContext& context,
                                                                           const ToolInvocation& invocation) {
    const auto student_id = json_id(invocation.arguments_json, "student_id");
    const auto course_id = json_id(invocation.arguments_json, "course_id");
    if (!student_id.ok() || !course_id.ok())
      return common::Result<ToolResult>::failure(common::ErrorCode::ToolArgumentError,
                                                 "Invalid student-progress tool arguments.");
    const auto result = progress.list(context.actor_id, *student_id.value, *course_id.value);
    return tool_result(result, result.ok() ? "{\"count\":" + std::to_string(result.value->size()) + "}" : "{}");
  });

  handlers.emplace(
      "get_question", [&questions = bindings.questions](const ToolContext& context, const ToolInvocation& invocation) {
        const auto question_id = json_id(invocation.arguments_json, "question_id");
        if (!question_id.ok())
          return common::Result<ToolResult>::failure(question_id.error->code, question_id.error->message);
        const auto result = questions.find(context.actor_id, *question_id.value);
        return tool_result(result, result.ok() ? "{\"id\":" + std::to_string(result.value->id) + ",\"question\":\"" +
                                                     escape_json(result.value->question) + "\"}"
                                               : "{}");
      });

  handlers.emplace("update_my_progress", [&progress = bindings.progress](const ToolContext& context,
                                                                         const ToolInvocation& invocation) {
    const auto course_id = json_id(invocation.arguments_json, "course_id");
    const auto topic = json_string(invocation.arguments_json, "topic");
    const auto status = json_string(invocation.arguments_json, "status");
    const auto value = json_number(invocation.arguments_json, "progress_value");
    if (!course_id.ok() || !topic.ok() || !status.ok() || !value.ok())
      return common::Result<ToolResult>::failure(common::ErrorCode::ToolArgumentError,
                                                 "Invalid progress tool arguments.");
    const auto result = progress.update(context.actor_id, {.user_id = context.actor_id,
                                                           .course_id = *course_id.value,
                                                           .topic = *topic.value,
                                                           .status = progress_status(*status.value),
                                                           .progress_value = *value.value});
    return tool_result(result, result.ok() ? "{\"id\":" + std::to_string(result.value->id) + "}" : "{}");
  });

  handlers.emplace("get_course_assessments", [&assessments = bindings.assessments](const ToolContext& context,
                                                                                   const ToolInvocation& invocation) {
    const auto course_id = json_id(invocation.arguments_json, "course_id");
    if (!course_id.ok())
      return common::Result<ToolResult>::failure(course_id.error->code, course_id.error->message);
    const auto result = assessments.list_for_course(context.actor_id, *course_id.value);
    return tool_result(result, result.ok() ? "{\"count\":" + std::to_string(result.value->size()) + "}" : "{}");
  });

  handlers.emplace("get_teacher_policy", [&configurations = bindings.teacher_configurations](
                                             const ToolContext& context, const ToolInvocation& invocation) {
    const auto course_id = json_id(invocation.arguments_json, "course_id");
    if (!course_id.ok())
      return common::Result<ToolResult>::failure(course_id.error->code, course_id.error->message);
    const auto result = configurations.get(context.actor_id, context.actor_id, *course_id.value);
    return tool_result(result, result.ok() ? "{\"id\":" + std::to_string(result.value->id) + "}" : "{}");
  });

  handlers.emplace("save_teacher_policy", [&configurations = bindings.teacher_configurations](
                                              const ToolContext& context, const ToolInvocation& invocation) {
    const auto course_id = json_id(invocation.arguments_json, "course_id");
    const auto policy_text = json_string(invocation.arguments_json, "policy_text");
    if (!course_id.ok() || !policy_text.ok())
      return common::Result<ToolResult>::failure(common::ErrorCode::ToolArgumentError,
                                                 "Invalid teacher-policy tool arguments.");
    const auto result = configurations.save(
        context.actor_id,
        {.teacher_id = context.actor_id, .course_id = *course_id.value, .policy_text = *policy_text.value});
    return tool_result(result, result.ok() ? "{\"id\":" + std::to_string(result.value->id) + "}" : "{}");
  });

  handlers.emplace("add_question", [&questions = bindings.questions](const ToolContext& context,
                                                                        const ToolInvocation& invocation) {
    const auto course_id = json_id(invocation.arguments_json, "course_id");
    const auto question_text = json_string(invocation.arguments_json, "question");
    const auto answer = json_string(invocation.arguments_json, "answer");
    const auto hint = json_optional_string(invocation.arguments_json, "hint");
    const auto difficulty = json_optional_string(invocation.arguments_json, "difficulty");
    const auto question_type = json_optional_string(invocation.arguments_json, "question_type");
    const auto source = json_optional_string(invocation.arguments_json, "source");
    if (!course_id.ok() || !question_text.ok() || !answer.ok() || !hint.ok() || !difficulty.ok() ||
        !question_type.ok() || !source.ok())
      return common::Result<ToolResult>::failure(common::ErrorCode::ToolArgumentError,
                                                 "Invalid add-question tool arguments.");
    const auto result = questions.create(context.actor_id,
                                         {.course_id = *course_id.value,
                                          .question = *question_text.value,
                                          .answer = *answer.value,
                                          .hint = hint.value->value_or(""),
                                          .difficulty = difficulty.value->value_or(""),
                                          .question_type = question_type.value->value_or(""),
                                          .source = source.value->value_or("")});
    return tool_result(result, result.ok() ? "{\"id\":" + std::to_string(result.value->id) + "}" : "{}");
  });

  handlers.emplace("add_thinking_assessment", [&assessments = bindings.assessments](const ToolContext& context,
                                                                                       const ToolInvocation& invocation) {
    const auto student_id = json_id(invocation.arguments_json, "student_id");
    const auto course_id = json_id(invocation.arguments_json, "course_id");
    const auto message_id = json_id(invocation.arguments_json, "message_id");
    const auto score = json_number(invocation.arguments_json, "score");
    const auto dimensions_json = json_string(invocation.arguments_json, "dimensions_json");
    const auto reasoning = json_string(invocation.arguments_json, "reasoning");
    const auto model_name = json_string(invocation.arguments_json, "model_name");
    if (!student_id.ok() || !course_id.ok() || !message_id.ok() || !score.ok() || !dimensions_json.ok() ||
        !reasoning.ok() || !model_name.ok())
      return common::Result<ToolResult>::failure(common::ErrorCode::ToolArgumentError,
                                                 "Invalid add-thinking-assessment tool arguments.");
    const auto result = assessments.create(context.actor_id,
                                           {.user_id = *student_id.value,
                                            .course_id = *course_id.value,
                                            .message_id = *message_id.value,
                                            .score = *score.value,
                                            .dimensions_json = *dimensions_json.value,
                                            .reasoning = *reasoning.value,
                                            .model_name = *model_name.value});
    return tool_result(result, result.ok() ? "{\"id\":" + std::to_string(result.value->id) + "}" : "{}");
  });

  handlers.emplace("get_my_thinking_assessments", [&assessments = bindings.assessments](
                                                     const ToolContext& context,
                                                     const ToolInvocation& invocation) {
    const auto course_id = json_id(invocation.arguments_json, "course_id");
    if (!course_id.ok())
      return common::Result<ToolResult>::failure(course_id.error->code, course_id.error->message);
    const auto result = assessments.list_for_self(context.actor_id, *course_id.value);
    return tool_result(result, result.ok() ? "{\"count\":" + std::to_string(result.value->size()) + "}" : "{}");
  });

  handlers.emplace("review_thinking_assessment", [&assessments = bindings.assessments](
                                                 const ToolContext& context,
                                                 const ToolInvocation& invocation) {
    const auto assessment_id = json_id(invocation.arguments_json, "assessment_id");
    const auto teacher_score = json_number(invocation.arguments_json, "teacher_score");
    const auto teacher_feedback = json_optional_string(invocation.arguments_json, "teacher_feedback");
    const auto review_status = json_optional_string(invocation.arguments_json, "review_status");
    if (!assessment_id.ok() || !teacher_score.ok() || !teacher_feedback.ok() || !review_status.ok())
      return common::Result<ToolResult>::failure(common::ErrorCode::ToolArgumentError,
                                                 "Invalid thinking-assessment review arguments.");
    const auto result = assessments.review(context.actor_id,
                                           {.id = *assessment_id.value,
                                            .teacher_score = *teacher_score.value,
                                            .teacher_feedback = *teacher_feedback.value,
                                            .review_status = review_status.value->value_or("reviewed")});
    return tool_result(result, result.ok() ? "{\"id\":" + std::to_string(result.value->id) + "}" : "{}");
  });

  if (handlers.size() != definitions.size())
    return common::Result<std::size_t>::failure(common::ErrorCode::InvalidRequest,
                                                "Tool catalog and service handler map do not match.");

  std::size_t count{};
  for (const auto& [name, definition] : definitions) {
    const auto handler = handlers.find(name);
    if (handler == handlers.end() || !registry.register_tool(definition, handler->second))
      return common::Result<std::size_t>::failure(common::ErrorCode::Conflict, "Unable to register tool: " + name);
    ++count;
  }
  return common::Result<std::size_t>::success(count);
}
}  // namespace edu_ai::ai
