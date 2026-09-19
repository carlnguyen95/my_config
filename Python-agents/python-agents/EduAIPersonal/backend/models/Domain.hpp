#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace edu_ai::models {

using Id = std::int64_t;

enum class Role { Student, Teacher, Admin };
enum class AnswerPolicy { FullAnswer, HintOnly, Guided };
enum class QuestionStatus { Draft, Review, Approved, Archived };
enum class ProgressStatus { NotStarted, Learning, Completed, NeedsReview };

struct User { Id id{}; std::string name; std::string email; std::string password_hash; Role role{Role::Student}; std::string status{"active"}; };
struct Subject { Id id{}; std::string name; std::string description; };
struct Course { Id id{}; Id subject_id{}; std::string name; std::string description; Id teacher_id{}; AnswerPolicy answer_policy{AnswerPolicy::Guided}; };
struct Enrollment { Id id{}; Id user_id{}; Id course_id{}; };
struct LearningSession { Id id{}; Id user_id{}; Id course_id{}; };
struct Message { Id id{}; Id session_id{}; std::string role; std::string content; std::optional<int> token_input; std::optional<int> token_output; std::string model; };
struct Question { Id id{}; Id course_id{}; Id created_by{}; std::string question; std::string answer; std::string hint; std::string difficulty; std::string question_type; std::string source; QuestionStatus status{QuestionStatus::Draft}; };
struct Document { Id id{}; Id course_id{}; std::string title; std::string source_path; };
struct DocumentChunk { Id id{}; Id document_id{}; int chunk_index{}; std::string content; std::string embedding; };
struct Roadmap { Id id{}; Id user_id{}; Id course_id{}; std::string title; std::string content_json; };
struct LearningProgress { Id id{}; Id user_id{}; Id course_id{}; std::string topic; ProgressStatus status{ProgressStatus::NotStarted}; double progress_value{}; };
struct ThinkingAssessment { Id id{}; Id user_id{}; Id course_id{}; Id message_id{}; double score{}; std::string dimensions_json; std::string reasoning; std::string model_name; std::optional<double> teacher_score; std::optional<std::string> teacher_feedback; std::string review_status{"pending"}; };

}  // namespace edu_ai::models

