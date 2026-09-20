#include "backend/repositories/RoadmapRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

/**
 * @brief Finds a roadmap by its ID.
 * @param id Roadmap ID.
 * @return Roadmap or no value.
 */
std::optional<Roadmap> SqliteRoadmapRepository::find_by_id(Id id) {
  auto s = prepare(db_, "SELECT id,user_id,course_id,title,content_json FROM roadmaps WHERE id=?;");
  bind(s.get(), 1, id);
  if (sqlite3_step(s.get()) != SQLITE_ROW)
    return std::nullopt;
  return Roadmap{.id = sqlite3_column_int64(s.get(), 0),
                 .user_id = sqlite3_column_int64(s.get(), 1),
                 .course_id = sqlite3_column_int64(s.get(), 2),
                 .title = text(s.get(), 3),
                 .content_json = text(s.get(), 4)};
}

/**
 * @brief Finds a roadmap.
 * @param user User ID.
 * @param course Course ID.
 * @return Roadmap or no value.
 */
std::optional<Roadmap> SqliteRoadmapRepository::find_for_user_course(Id user, Id course) {
  auto s = prepare(db_,
                   "SELECT id,user_id,course_id,title,content_json FROM roadmaps WHERE user_id=? AND course_id=? ORDER "
                   "BY id DESC LIMIT 1;");
  bind(s.get(), 1, user);
  bind(s.get(), 2, course);
  if (sqlite3_step(s.get()) != SQLITE_ROW)
    return std::nullopt;
  return Roadmap{.id = sqlite3_column_int64(s.get(), 0),
                 .user_id = sqlite3_column_int64(s.get(), 1),
                 .course_id = sqlite3_column_int64(s.get(), 2),
                 .title = text(s.get(), 3),
                 .content_json = text(s.get(), 4)};
}

/**
 * @brief Saves a roadmap.
 * @param r Roadmap fields.
 * @return Persisted roadmap.
 */
Roadmap SqliteRoadmapRepository::save(Roadmap r) {
  const auto now = utc_now();
  if (r.id == 0) {
    auto s = prepare(
        db_, "INSERT INTO roadmaps(user_id,course_id,title,content_json,created_at,updated_at) VALUES(?,?,?,?,?,?);");
    bind(s.get(), 1, r.user_id);
    bind(s.get(), 2, r.course_id);
    bind(s.get(), 3, r.title);
    bind(s.get(), 4, r.content_json);
    bind(s.get(), 5, now);
    bind(s.get(), 6, now);
    done(s.get());
    r.id = sqlite3_last_insert_rowid(db_.handle());
  } else {
    auto s = prepare(db_, "UPDATE roadmaps SET title=?,content_json=?,updated_at=? WHERE id=?;");
    bind(s.get(), 1, r.title);
    bind(s.get(), 2, r.content_json);
    bind(s.get(), 3, now);
    bind(s.get(), 4, r.id);
    done(s.get());
  }
  return r;
}

/**
 * @brief Deletes a roadmap.
 * @param id Roadmap ID.
 * @return True when deleted.
 */
bool SqliteRoadmapRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM roadmaps WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
