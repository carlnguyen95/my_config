#include "backend/ai/AIEngine.hpp"

namespace edu_ai::ai {
AIResponse AIEngine::process(const LearningRequest& request) {
  PromptContext context; context.role = request.role; context.answer_policy = policies_.resolve_answer_policy(request.role, request.course.answer_policy); context.course_context = request.course.name + "\n" + request.course.description; context.history = request.history; context.user_request = request.user_message;
  // Future implementation: retrieve RAG context, call provider, validate each tool call,
  // execute authorized tools, call provider again, then persist the interaction.
  return provider_.generate({.messages = prompts_.build_learning_messages(context)});
}
}  // namespace edu_ai::ai

