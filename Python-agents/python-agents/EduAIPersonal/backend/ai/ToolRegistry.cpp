#include "backend/ai/ToolRegistry.hpp"

namespace edu_ai::ai {
namespace { bool has_role(models::Role actual, models::Role required) { return actual == models::Role::Admin || actual == required || required == models::Role::Student; } }
bool ToolRegistry::register_tool(ToolDefinition definition, ToolHandler handler) {
  return tools_.emplace(definition.name, RegisteredTool{std::move(definition), std::move(handler)}).second;
}
common::Result<std::string> ToolRegistry::execute(const std::string& name, models::Id actor_id, models::Role role, const std::string& arguments_json) const {
  const auto it = tools_.find(name);
  if (it == tools_.end()) return common::Result<std::string>::failure(common::ErrorCode::ToolNotFound, "Tool is not registered.");
  if (!has_role(role, it->second.definition.minimum_role)) return common::Result<std::string>::failure(common::ErrorCode::ToolPermissionDenied, "Role is not allowed to invoke this tool.");
  // JSON-schema validation belongs here before handler execution in the implementation phase.
  return it->second.handler(actor_id, arguments_json);
}
std::optional<ToolDefinition> ToolRegistry::find_definition(const std::string& name) const { const auto it = tools_.find(name); return it == tools_.end() ? std::nullopt : std::optional<ToolDefinition>{it->second.definition}; }
}  // namespace edu_ai::ai

