#include "backend/ai/ToolRegistry.hpp"
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
  CHECK(suite, catalog.ok() && catalog.value->size() == 13);
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
  return suite.finish();
}
