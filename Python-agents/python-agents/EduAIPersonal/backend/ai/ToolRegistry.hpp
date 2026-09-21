#pragma once

#include <functional>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "backend/common/Result.hpp"
#include "backend/models/Domain.hpp"

namespace edu_ai::services {
class AssessmentService;
class CourseService;
class ProgressService;
class QuestionService;
class RoadmapService;
class TeacherConfigurationService;
}  // namespace edu_ai::services

namespace edu_ai::repositories {
class DocumentChunkRepository;
}

namespace edu_ai::ai {

enum class ToolMode { Read, Write };
enum class ToolExecutionStatus { Success, NeedsConfirmation };

struct ToolDefinition {
  std::string name;
  std::string description;
  std::string argument_schema_json;
  models::Role minimum_role{models::Role::Student};
  ToolMode mode{ToolMode::Read};
  std::string confirmation_summary;
};

struct ToolContext {
  models::Id actor_id{};
  models::Role role{models::Role::Student};
  bool write_confirmed{false};
};

struct ToolInvocation {
  std::string call_id;
  std::string name;
  std::string arguments_json;
};

struct ToolResult {
  std::string call_id;
  ToolExecutionStatus status{ToolExecutionStatus::Success};
  std::string data_json{"{}"};
  std::string confirmation_summary;
};

using ToolHandler = std::function<common::Result<ToolResult>(const ToolContext&, const ToolInvocation&)>;

struct RegisteredTool {
  ToolDefinition definition;
  ToolHandler handler;
};

struct ServiceToolBindings {
  services::CourseService& courses;
  services::RoadmapService& roadmaps;
  services::ProgressService& progress;
  services::QuestionService& questions;
  services::AssessmentService& assessments;
  services::TeacherConfigurationService& teacher_configurations;
  repositories::DocumentChunkRepository* document_chunks{nullptr};
};

class ToolRegistry {
 public:
  /// Registers a unique tool definition and its trusted server-side handler.
  bool register_tool(ToolDefinition definition, ToolHandler handler);
  /// Authorizes and runs a tool invocation, requesting confirmation for writes.
  common::Result<ToolResult> execute(const ToolContext& context, const ToolInvocation& invocation) const;
  /// Returns a registered tool definition by name without exposing its handler.
  std::optional<ToolDefinition> find_definition(const std::string& name) const;
  /// Returns the definitions that can be advertised to an AI provider.
  std::vector<ToolDefinition> list_definitions() const;

 private:
  std::unordered_map<std::string, RegisteredTool> tools_;
};

/// Loads tool metadata from the JSON catalog and validates its required fields.
common::Result<std::vector<ToolDefinition>> load_tool_definitions(const std::filesystem::path& catalog_path);
/// Registers catalog definitions with their matching production service handlers.
common::Result<std::size_t> register_available_tools(ToolRegistry& registry, ServiceToolBindings bindings,
                                                     const std::filesystem::path& catalog_path);
}  // namespace edu_ai::ai
