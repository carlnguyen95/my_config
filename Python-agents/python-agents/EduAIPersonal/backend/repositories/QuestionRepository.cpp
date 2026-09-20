#include "backend/repositories/QuestionRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

namespace {
/**
 * @brief Maps selected columns to a question.
 * @param s SQLite row.
 * @return Domain question.
 */
Question row(sqlite3_stmt* s) {
  return {.id = sqlite3_column_int64(s, 0),
          .course_id = sqlite3_column_int64(s, 1),
          .created_by = sqlite3_column_int64(s, 2),
          .question = text(s, 3),
          .answer = text(s, 4),
          .hint = text(s, 5),
          .difficulty = text(s, 6),
          .question_type = text(s, 7),
          .source = text(s, 8),
          .status = question_status_from(text(s, 9))};
}
}  // namespace

/**
 * @brief Searches course questions.
 * @param course Course ID.
 * @param query Search text.
 * @param approved Approval filter.
 * @return Questions.
 */
std::vector<Question> SqliteQuestionRepository::search(Id course, const std::string& query, bool approved) {
  auto s = prepare(
      db_, approved ? "SELECT id,course_id,created_by,question,answer,hint,difficulty,question_type,source,status FROM "
                      "questions WHERE course_id=? AND status='approved' AND question LIKE ? ORDER BY id DESC;"
                    : "SELECT id,course_id,created_by,question,answer,hint,difficulty,question_type,source,status FROM "
                      "questions WHERE course_id=? AND question LIKE ? ORDER BY id DESC;");
  bind(s.get(), 1, course);
  bind(s.get(), 2, "%" + query + "%");
  std::vector<Question> r;
  while (sqlite3_step(s.get()) == SQLITE_ROW) r.push_back(row(s.get()));
  return r;
}

/**
 * @brief Finds a question.
 * @param id Question ID.
 * @return Question or no value.
 */
std::optional<Question> SqliteQuestionRepository::find_by_id(Id id) {
  auto s = prepare(db_,
                   "SELECT id,course_id,created_by,question,answer,hint,difficulty,question_type,source,status FROM "
                   "questions WHERE id=?;");
  bind(s.get(), 1, id);
  return sqlite3_step(s.get()) == SQLITE_ROW ? std::optional<Question>{row(s.get())} : std::nullopt;
}

/**
 * @brief Saves a question.
 * @param q Question fields.
 * @return Persisted question.
 */
Question SqliteQuestionRepository::save(Question q) {
  const auto now = utc_now();
  if (q.id == 0) {
    auto s = prepare(db_,
                     "INSERT INTO "
                     "questions(course_id,created_by,question,answer,hint,difficulty,question_type,source,status,"
                     "created_at,updated_at) VALUES(?,?,?,?,?,?,?,?,?,?,?);");
    bind(s.get(), 1, q.course_id);
    bind(s.get(), 2, q.created_by);
    bind(s.get(), 3, q.question);
    bind(s.get(), 4, q.answer);
    bind(s.get(), 5, q.hint);
    bind(s.get(), 6, q.difficulty);
    bind(s.get(), 7, q.question_type);
    bind(s.get(), 8, q.source);
    bind(s.get(), 9, question_status_to(q.status));
    bind(s.get(), 10, now);
    bind(s.get(), 11, now);
    done(s.get());
    q.id = sqlite3_last_insert_rowid(db_.handle());
  } else {
    auto s =
        prepare(db_,
                "UPDATE questions SET "
                "question=?,answer=?,hint=?,difficulty=?,question_type=?,source=?,status=?,updated_at=? WHERE id=?;");
    bind(s.get(), 1, q.question);
    bind(s.get(), 2, q.answer);
    bind(s.get(), 3, q.hint);
    bind(s.get(), 4, q.difficulty);
    bind(s.get(), 5, q.question_type);
    bind(s.get(), 6, q.source);
    bind(s.get(), 7, question_status_to(q.status));
    bind(s.get(), 8, now);
    bind(s.get(), 9, q.id);
    done(s.get());
  }
  return q;
}

using namespace sqlite_detail;

/**
 * @brief Filters questions by status.
 * @param course_id Course ID.
 * @param status Workflow status.
 * @return Questions.
 */
std::vector<Question> SqliteQuestionRepository::find_by_status(Id course_id, QuestionStatus status) {
  auto s = prepare(db_,
                   "SELECT id,course_id,created_by,question,answer,hint,difficulty,question_type,source,status FROM "
                   "questions WHERE course_id=? AND status=? ORDER BY id DESC;");
  bind(s.get(), 1, course_id);
  bind(s.get(), 2, question_status_to(status));
  std::vector<Question> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(row(s.get()));
  return result;
}

/**
 * @brief Searches question content.
 * @param course_id Course ID.
 * @param content Search text.
 * @return Questions.
 */
std::vector<Question> SqliteQuestionRepository::find_by_content(Id course_id, const std::string& content) {
  return search(course_id, content, false);
}

/**
 * @brief Filters questions by timestamp.
 * @param course_id Course ID.
 * @param created_at UTC timestamp.
 * @return Questions.
 */
std::vector<Question> SqliteQuestionRepository::find_by_created_at(Id course_id, const std::string& created_at) {
  auto s = prepare(db_,
                   "SELECT id,course_id,created_by,question,answer,hint,difficulty,question_type,source,status FROM "
                   "questions WHERE course_id=? AND created_at=? ORDER BY id DESC;");
  bind(s.get(), 1, course_id);
  bind(s.get(), 2, created_at);
  std::vector<Question> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(row(s.get()));
  return result;
}

/**
 * @brief Archives a question.
 * @param id Question ID.
 * @return True when updated.
 */
bool SqliteQuestionRepository::archive(Id id) {
  auto s = prepare(db_, "UPDATE questions SET status='archived', updated_at=? WHERE id=?;");
  bind(s.get(), 1, utc_now());
  bind(s.get(), 2, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}

/**
 * @brief Permanently deletes a question.
 * @param id Question primary key.
 * @return True when one question was removed.
 */
bool SqliteQuestionRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM questions WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
