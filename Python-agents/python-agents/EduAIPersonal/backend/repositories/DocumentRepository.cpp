#include "backend/repositories/DocumentRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

std::vector<Document> SqliteDocumentRepository::find_by_course(Id course_id) {
  auto s = prepare(db_, "SELECT id, course_id, title, source_path FROM documents WHERE course_id=? ORDER BY created_at DESC;");
  bind(s.get(), 1, course_id);
  std::vector<Document> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) {
    result.push_back({.id = sqlite3_column_int64(s.get(), 0),
                      .course_id = sqlite3_column_int64(s.get(), 1),
                      .title = text(s.get(), 2),
                      .source_path = text(s.get(), 3)});
  }
  return result;
}

/**
 * @brief Saves a source document.
 * @param d Document fields.
 * @return Persisted document.
 */
Document SqliteDocumentRepository::save(Document d) {
  if (d.id == 0) {
    auto s = prepare(db_, "INSERT INTO documents(course_id,title,source_path,created_at) VALUES(?,?,?,?);");
    bind(s.get(), 1, d.course_id);
    bind(s.get(), 2, d.title);
    bind(s.get(), 3, d.source_path);
    bind(s.get(), 4, utc_now());
    done(s.get());
    d.id = sqlite3_last_insert_rowid(db_.handle());
  }
  return d;
}

/**
 * @brief Deletes a source document.
 * @param id Document ID.
 * @return True when deleted.
 */
bool SqliteDocumentRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM documents WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
