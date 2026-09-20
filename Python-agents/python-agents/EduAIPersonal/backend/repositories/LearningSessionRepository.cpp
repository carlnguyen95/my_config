#include "backend/repositories/LearningSessionRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

/**
 * @brief Finds a learning session.
 * @param id Session ID.
 * @return Session or no value.
 */
std::optional<LearningSession> SqliteLearningSessionRepository::find_by_id(Id id) {
  auto s = prepare(db_, "SELECT id,user_id,course_id FROM learning_sessions WHERE id=?;");
  bind(s.get(), 1, id);
  if (sqlite3_step(s.get()) != SQLITE_ROW)
    return std::nullopt;
  return LearningSession{.id = sqlite3_column_int64(s.get(), 0),
                         .user_id = sqlite3_column_int64(s.get(), 1),
                         .course_id = sqlite3_column_int64(s.get(), 2)};
}

/**
 * @brief Saves a learning session.
 * @param session Session fields.
 * @return Persisted session.
 */
LearningSession SqliteLearningSessionRepository::save(LearningSession session) {
  if (session.id == 0) {
    auto s = prepare(db_, "INSERT INTO learning_sessions(user_id,course_id,started_at) VALUES(?,?,?);");
    bind(s.get(), 1, session.user_id);
    bind(s.get(), 2, session.course_id);
    bind(s.get(), 3, utc_now());
    done(s.get());
    session.id = sqlite3_last_insert_rowid(db_.handle());
  }
  return session;
}

/**
 * @brief Deletes a learning session.
 * @param id Session ID.
 * @return True when deleted.
 */
bool SqliteLearningSessionRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM learning_sessions WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
