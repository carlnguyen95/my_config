#include "backend/repositories/ProgressRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

/**
 * @brief Lists progress.
 * @param user User ID.
 * @param course Course ID.
 * @return Progress entries.
 */
std::vector<LearningProgress> SqliteProgressRepository::list(Id user, Id course) {
  auto s = prepare(db_,
                   "SELECT id,user_id,course_id,topic,status,progress_value FROM learning_progress WHERE user_id=? AND "
                   "course_id=? ORDER BY topic;");
  bind(s.get(), 1, user);
  bind(s.get(), 2, course);
  std::vector<LearningProgress> r;
  while (sqlite3_step(s.get()) == SQLITE_ROW)
    r.push_back({.id = sqlite3_column_int64(s.get(), 0),
                 .user_id = sqlite3_column_int64(s.get(), 1),
                 .course_id = sqlite3_column_int64(s.get(), 2),
                 .topic = text(s.get(), 3),
                 .status = progress_status_from(text(s.get(), 4)),
                 .progress_value = sqlite3_column_double(s.get(), 5)});
  return r;
}

/**
 * @brief Saves progress.
 * @param p Progress fields.
 * @return Persisted progress.
 */
LearningProgress SqliteProgressRepository::save(LearningProgress p) {
  const auto now = utc_now();
  if (p.id == 0) {
    auto s = prepare(db_,
                     "INSERT INTO learning_progress(user_id,course_id,topic,status,progress_value,updated_at) "
                     "VALUES(?,?,?,?,?,?) ON CONFLICT(user_id,course_id,topic) DO UPDATE SET "
                     "status=excluded.status,progress_value=excluded.progress_value,updated_at=excluded.updated_at;");
    bind(s.get(), 1, p.user_id);
    bind(s.get(), 2, p.course_id);
    bind(s.get(), 3, p.topic);
    bind(s.get(), 4, progress_status_to(p.status));
    bind(s.get(), 5, p.progress_value);
    bind(s.get(), 6, now);
    done(s.get());
    auto f = prepare(db_, "SELECT id FROM learning_progress WHERE user_id=? AND course_id=? AND topic=?;");
    bind(f.get(), 1, p.user_id);
    bind(f.get(), 2, p.course_id);
    bind(f.get(), 3, p.topic);
    if (sqlite3_step(f.get()) == SQLITE_ROW)
      p.id = sqlite3_column_int64(f.get(), 0);
  } else {
    auto s = prepare(db_, "UPDATE learning_progress SET topic=?,status=?,progress_value=?,updated_at=? WHERE id=?;");
    bind(s.get(), 1, p.topic);
    bind(s.get(), 2, progress_status_to(p.status));
    bind(s.get(), 3, p.progress_value);
    bind(s.get(), 4, now);
    bind(s.get(), 5, p.id);
    done(s.get());
  }
  return p;
}

/**
 * @brief Deletes progress.
 * @param id Progress ID.
 * @return True when deleted.
 */
bool SqliteProgressRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM learning_progress WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
