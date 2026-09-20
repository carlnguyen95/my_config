#pragma once

#include <optional>
#include <string>

#include <json/json.h>

#include "backend/common/Result.hpp"
#include "backend/models/Domain.hpp"

namespace edu_ai::common {

/**
 * @brief Escapes text for the small JSON response payloads emitted by tools.
 * @param value Text to escape.
 * @return JSON-safe string text.
 */
std::string escape_json(const std::string& value);

/**
 * @brief Parses model-provided tool arguments into a JSON object.
 * @param json Model-provided JSON arguments.
 * @return Parsed object, or a tool-argument error.
 */
Result<Json::Value> json_object(const std::string& json);

/**
 * @brief Reads an integer field from a tool argument JSON object.
 * @param json Model-provided JSON arguments.
 * @param key Field name to read.
 * @return Parsed ID, or a tool-argument error.
 */
Result<models::Id> json_id(const std::string& json, const std::string& key);

/**
 * @brief Reads a string field from a tool argument JSON object.
 * @param json Model-provided JSON arguments.
 * @param key Field name to read.
 * @return Parsed string, or a tool-argument error.
 */
Result<std::string> json_string(const std::string& json, const std::string& key);

/**
 * @brief Reads an optional string field from a tool argument JSON object.
 * @param json Model-provided JSON arguments.
 * @param key Field name to read.
 * @return Parsed optional string, or a tool-argument error.
 */
Result<std::optional<std::string>> json_optional_string(const std::string& json, const std::string& key);

/**
 * @brief Reads a decimal field from a tool argument JSON object.
 * @param json Model-provided JSON arguments.
 * @param key Field name to read.
 * @return Parsed decimal, or a tool-argument error.
 */
Result<double> json_number(const std::string& json, const std::string& key);

}  // namespace edu_ai::common
