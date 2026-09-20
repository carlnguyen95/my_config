#include "backend/services/QuestionService.hpp"
#include "backend/services/ServiceSupport.hpp"

namespace edu_ai::services {
using namespace detail;

/**
 * @brief Returns a visible question.
 * @param actor_id Requester ID.
 * @param question_id Question ID.
 * @return Question or error.
 */
common::Result<Question> DefaultQuestionService::find(Id actor_id, Id question_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<Question>();
  const auto question = questions_.find_by_id(question_id);
  if (!question)
    return common::Result<Question>::failure(common::ErrorCode::NotFound, "Question was not found.");
  if (!can_access_course(*current, courses_, enrollments_, question->course_id))
    return denied<Question>();
  if (current->role == Role::Student && question->status != QuestionStatus::Approved)
    return common::Result<Question>::failure(common::ErrorCode::NotFound, "Question was not found.");
  return common::Result<Question>::success(*question);
}

/**
 * @brief Creates a managed course question.
 * @param actor_id Teacher ID.
 * @param question Question fields.
 * @return Saved question or error.
 */
common::Result<Question> DefaultQuestionService::create(Id actor_id, Question question) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<Question>();
  if (!can_manage_course(*current, courses_, question.course_id))
    return denied<Question>();
  if (question.question.empty() || question.answer.empty())
    return common::Result<Question>::failure(common::ErrorCode::InvalidRequest, "Question and answer are required.");
  question.created_by = current->id;
  if (question.source.empty())
    question.source = "teacher";
  if (question.difficulty.empty())
    question.difficulty = "medium";
  if (question.question_type.empty())
    question.question_type = "short_answer";
  return common::Result<Question>::success(questions_.save(std::move(question)));
}

/**
 * @brief Approves a managed question.
 * @param actor_id Teacher ID.
 * @param question_id Question ID.
 * @return Approved question or error.
 */
common::Result<Question> DefaultQuestionService::approve(Id actor_id, Id question_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<Question>();
  const auto question = questions_.find_by_id(question_id);
  if (!question)
    return common::Result<Question>::failure(common::ErrorCode::NotFound, "Question was not found.");
  if (!can_manage_course(*current, courses_, question->course_id))
    return denied<Question>();
  auto approved = *question;
  approved.status = QuestionStatus::Approved;
  return common::Result<Question>::success(questions_.save(std::move(approved)));
}

/**
 * @brief Archives a managed question.
 * @param actor_id Teacher ID.
 * @param question_id Question ID.
 * @return Operation result or error.
 */
common::Result<bool> DefaultQuestionService::archive(Id actor_id, Id question_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<bool>();
  const auto question = questions_.find_by_id(question_id);
  if (!question)
    return common::Result<bool>::failure(common::ErrorCode::NotFound, "Question was not found.");
  if (!can_manage_course(*current, courses_, question->course_id))
    return denied<bool>();
  return common::Result<bool>::success(questions_.archive(question_id));
}

/**
 * @brief Permanently removes a teacher-managed question.
 * @param actor_id Authenticated teacher ID.
 * @param question_id Question to remove.
 * @return Whether the question was removed, or an error.
 */
common::Result<bool> DefaultQuestionService::remove(Id actor_id, Id question_id) {
  const auto current = actor(users_, actor_id);
  const auto question = questions_.find_by_id(question_id);
  if (!current)
    return missing_actor<bool>();
  if (!question)
    return common::Result<bool>::failure(common::ErrorCode::NotFound, "Question was not found.");
  if (!can_manage_course(*current, courses_, question->course_id))
    return denied<bool>();
  return common::Result<bool>::success(questions_.remove(question_id));
}
}  // namespace edu_ai::services
