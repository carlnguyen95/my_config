#include <cassert>

#include "backend/ai/PolicyEngine.hpp"

int main() {
  edu_ai::ai::PolicyEngine engine;
  using edu_ai::models::AnswerPolicy;
  using edu_ai::models::Role;
  assert(engine.resolve_answer_policy(Role::Student, AnswerPolicy::HintOnly) == AnswerPolicy::HintOnly);
  assert(engine.resolve_answer_policy(Role::Teacher, AnswerPolicy::HintOnly) == AnswerPolicy::FullAnswer);
  assert(!engine.can_manage_questions(Role::Student));
  assert(engine.can_manage_questions(Role::Teacher));
}

