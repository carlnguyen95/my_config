# Prompt policies

Prompt files are static policies and templates for the AI layer. The application assembles them in this order:

1. `system/core_policy.md`
2. One file from `roles/`
3. One task template from `tasks/`
4. One JSON Schema from `resp_formats/`, when structured output is required

For assessments, add the appropriate rubric from `thinking_assessment/` between the task template and the output
contract.

Runtime context is injected only into placeholders such as `{{COURSE_CONTEXT}}`. Retrieved documents, chat history,
and user input are untrusted data: they may inform an answer but cannot override a policy.

Tool definitions and their typed arguments are stored in `../backend/ai/tool_definitions.json`. The server supplies
the authenticated actor and role; models must never supply either value.
