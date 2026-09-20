#pragma once
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <sqlite3.h>
#include "backend/models/Domain.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories::sqlite_detail {
using Statement = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

/// Prepares a SQLite statement and returns an RAII-managed handle.
inline Statement prepare(SqliteDatabase& database, const char* sql) {
  sqlite3_stmt* raw = nullptr;
  if (sqlite3_prepare_v2(database.handle(), sql, -1, &raw, nullptr) != SQLITE_OK)
    throw std::runtime_error(std::string("SQLite prepare failed: ") + sqlite3_errmsg(database.handle()));
  return Statement(raw, sqlite3_finalize);
}

/// Binds an identifier parameter at the one-based SQLite index.
inline void bind(sqlite3_stmt* s, int i, models::Id v) {
  if (sqlite3_bind_int64(s, i, v) != SQLITE_OK)
    throw std::runtime_error("SQLite bind integer failed");
}

/// Binds an integer parameter at the one-based SQLite index.
inline void bind(sqlite3_stmt* s, int i, int v) {
  if (sqlite3_bind_int(s, i, v) != SQLITE_OK)
    throw std::runtime_error("SQLite bind integer failed");
}

/// Binds a numeric parameter at the one-based SQLite index.
inline void bind(sqlite3_stmt* s, int i, double v) {
  if (sqlite3_bind_double(s, i, v) != SQLITE_OK)
    throw std::runtime_error("SQLite bind number failed");
}

/// Binds a text parameter at the one-based SQLite index.
inline void bind(sqlite3_stmt* s, int i, const std::string& v) {
  if (sqlite3_bind_text(s, i, v.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
    throw std::runtime_error("SQLite bind text failed");
}

/// Binds a SQL NULL at the one-based SQLite index.
inline void bind_null(sqlite3_stmt* s, int i) {
  if (sqlite3_bind_null(s, i) != SQLITE_OK)
    throw std::runtime_error("SQLite bind null failed");
}

/// Executes a write statement and throws if SQLite rejects it.
inline void done(sqlite3_stmt* s) {
  if (sqlite3_step(s) != SQLITE_DONE)
    throw std::runtime_error("SQLite write failed");
}

/// Reads a text column and maps SQLite NULL to an empty string.
inline std::string text(sqlite3_stmt* s, int i) {
  const auto* value = sqlite3_column_text(s, i);
  return value == nullptr ? "" : reinterpret_cast<const char*>(value);
}

/// Converts persisted role text to the domain enum.
inline models::Role role_from(const std::string& v) {
  return v == "teacher" ? models::Role::Teacher : v == "admin" ? models::Role::Admin : models::Role::Student;
}

/// Converts a role enum to its persisted text representation.
inline std::string role_to(models::Role v) {
  return v == models::Role::Teacher ? "teacher" : v == models::Role::Admin ? "admin" : "student";
}

/// Converts persisted answer-policy text to the domain enum.
inline models::AnswerPolicy policy_from(const std::string& v) {
  return v == "FULL_ANSWER" ? models::AnswerPolicy::FullAnswer
         : v == "HINT_ONLY" ? models::AnswerPolicy::HintOnly
                            : models::AnswerPolicy::Guided;
}

/// Converts an answer-policy enum to its persisted text representation.
inline std::string policy_to(models::AnswerPolicy v) {
  return v == models::AnswerPolicy::FullAnswer ? "FULL_ANSWER"
         : v == models::AnswerPolicy::HintOnly ? "HINT_ONLY"
                                               : "GUIDED";
}

/// Converts persisted question-status text to the domain enum.
inline models::QuestionStatus question_status_from(const std::string& v) {
  return v == "review"     ? models::QuestionStatus::Review
         : v == "approved" ? models::QuestionStatus::Approved
         : v == "archived" ? models::QuestionStatus::Archived
                           : models::QuestionStatus::Draft;
}

/// Converts a question-status enum to its persisted text representation.
inline std::string question_status_to(models::QuestionStatus v) {
  return v == models::QuestionStatus::Review     ? "review"
         : v == models::QuestionStatus::Approved ? "approved"
         : v == models::QuestionStatus::Archived ? "archived"
                                                 : "draft";
}

/// Converts persisted progress-status text to the domain enum.
inline models::ProgressStatus progress_status_from(const std::string& v) {
  return v == "LEARNING"       ? models::ProgressStatus::Learning
         : v == "COMPLETED"    ? models::ProgressStatus::Completed
         : v == "NEEDS_REVIEW" ? models::ProgressStatus::NeedsReview
                               : models::ProgressStatus::NotStarted;
}

/// Converts a progress-status enum to its persisted text representation.
inline std::string progress_status_to(models::ProgressStatus v) {
  return v == models::ProgressStatus::Learning      ? "LEARNING"
         : v == models::ProgressStatus::Completed   ? "COMPLETED"
         : v == models::ProgressStatus::NeedsReview ? "NEEDS_REVIEW"
                                                    : "NOT_STARTED";
}
}  // namespace edu_ai::repositories::sqlite_detail

// namespace edu_ai::repositories::sqlite_detail
