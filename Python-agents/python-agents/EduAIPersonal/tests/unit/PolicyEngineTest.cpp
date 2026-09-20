#include "backend/ai/PolicyEngine.hpp"
#include "backend/ai/PromptBuilder.hpp"
#include "tests/common/TestSupport.hpp"

/**
 * @brief Exercises policy decisions.
 * @return Process exit code.
 */
int main() {
  edu_ai::tests::Suite suite("policy_engine");
  edu_ai::ai::PolicyEngine engine;
  using edu_ai::models::AnswerPolicy;
  using edu_ai::models::Role;
  suite.scenario("Student with HINT_ONLY course policy receives HINT_ONLY");
  CHECK(suite, engine.resolve_answer_policy(Role::Student, AnswerPolicy::HintOnly) == AnswerPolicy::HintOnly);
  suite.scenario("Teacher always receives FULL_ANSWER regardless of course policy");
  CHECK(suite, engine.resolve_answer_policy(Role::Teacher, AnswerPolicy::HintOnly) == AnswerPolicy::FullAnswer);
  suite.scenario("Teacher-only Question Bank permission is enforced");
  CHECK(suite, !engine.can_manage_questions(Role::Student));
  CHECK(suite, engine.can_manage_questions(Role::Teacher));
  edu_ai::ai::PromptBuilder prompts;
  edu_ai::ai::PromptContext context{.role = Role::Student,
                                    .answer_policy = AnswerPolicy::Guided,
                                    .course_context = "Networks",
                                    .teacher_policy = "Ask for an attempt before providing feedback.",
                                    .user_request = "Explain congestion control."};
  suite.scenario("Teacher course policy is included as scoped guidance in the model prompt");
  CHECK(suite,
        prompts.build_learning_messages(context).front().content.find(context.teacher_policy) != std::string::npos);
  return suite.finish();
}
