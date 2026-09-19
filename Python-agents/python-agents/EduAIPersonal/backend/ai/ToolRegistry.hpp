#pragma once

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "backend/common/Result.hpp"
#include "backend/models/Domain.hpp"

namespace edu_ai::ai {
struct ToolDefinition { std::string name; std::string description; std::string argument_schema_json; models::Role minimum_role{models::Role::Student}; };
using ToolHandler = std::function<common::Result<std::string>(models::Id actor_id, const std::string& arguments_json)>;
struct RegisteredTool { ToolDefinition definition; ToolHandler handler; };

class ToolRegistry {
 public:
  bool register_tool(ToolDefinition definition, ToolHandler handler);
  common::Result<std::string> execute(const std::string& name, models::Id actor_id, models::Role role, const std::string& arguments_json) const;
  std::optional<ToolDefinition> find_definition(const std::string& name) const;
 private:
  std::unordered_map<std::string, RegisteredTool> tools_;
};
}  // namespace edu_ai::ai

