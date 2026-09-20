# JSON response and tool contracts

Use these files as JSON Schema contracts, not as natural-language prompt fragments.

- `learning_response.schema.json`: final learner-facing answer after any tool calls finish.
- `assessment_response.schema.json`: AI assessment draft for teacher review.
- `tool_result.schema.json`: normalized tool execution result returned to the model.

Each model response has a `tool_calls` array. Send the selected item from `../../backend/ai/tool_definitions.json` in the provider's
tool-definition field when native function calling is available. When it is unavailable, parse `tool_calls` from the
response JSON. Return an empty array when no tool is needed.

`actor_id`, authenticated role, and confirmation state come from the server. They are never model-controlled
arguments.
