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

/**
 * @brief Parses model-provided tool arguments into a JSON object.
 * @param json Model-provided JSON arguments.
 * @return Parsed object, or a tool-argument error.
 */
Result<Json::Value> json_object(const std::string& json) {
  Json::Value object;
  Json::CharReaderBuilder builder;
  std::string errors;
  std::istringstream input(json);
  if (!Json::parseFromStream(builder, input, &object, &errors) || !object.isObject())
    return Result<Json::Value>::failure(ErrorCode::ToolArgumentError, "Tool arguments must be a JSON object.");
  return Result<Json::Value>::success(std::move(object));
}

/**
 * @brief Reads an integer field from a tool argument JSON object.
 * @param json Model-provided JSON arguments.
 * @param key Field name to read.
 * @return Parsed ID, or a tool-argument error.
 */
Result<models::Id> json_id(const std::string& json, const std::string& key) {
  const auto object = json_object(json);
  if (!object.ok())
    return Result<models::Id>::failure(object.error->code, object.error->message);
  if (!object.value->isMember(key) || !(*object.value)[key].isInt64())
    return Result<models::Id>::failure(ErrorCode::ToolArgumentError, "Missing integer argument: " + key);
  return Result<models::Id>::success((*object.value)[key].asInt64());
}

/**
 * @brief Reads a string field from a tool argument JSON object.
 * @param json Model-provided JSON arguments.
 * @param key Field name to read.
 * @return Parsed string, or a tool-argument error.
 */
Result<std::string> json_string(const std::string& json, const std::string& key) {
  const auto object = json_object(json);
  if (!object.ok())
    return Result<std::string>::failure(object.error->code, object.error->message);
  if (!object.value->isMember(key) || !(*object.value)[key].isString())
    return Result<std::string>::failure(ErrorCode::ToolArgumentError, "Missing string argument: " + key);
  return Result<std::string>::success((*object.value)[key].asString());
}

/**
 * @brief Reads an optional string field from a tool argument JSON object.
 * @param json Model-provided JSON arguments.
 * @param key Field name to read.
 * @return Parsed optional string, or a tool-argument error.
 */
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

/**
 * @brief Reads a decimal field from a tool argument JSON object.
 * @param json Model-provided JSON arguments.
 * @param key Field name to read.
 * @return Parsed decimal, or a tool-argument error.
 */
Result<double> json_number(const std::string& json, const std::string& key) {
  const auto object = json_object(json);
  if (!object.ok())
    return Result<double>::failure(object.error->code, object.error->message);
  if (!object.value->isMember(key) || !(*object.value)[key].isNumeric())
    return Result<double>::failure(ErrorCode::ToolArgumentError, "Missing numeric argument: " + key);
  return Result<double>::success((*object.value)[key].asDouble());
}

}  // namespace edu_ai::common
