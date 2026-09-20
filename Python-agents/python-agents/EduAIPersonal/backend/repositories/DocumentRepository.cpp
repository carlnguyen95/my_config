#include "backend/repositories/DocumentRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

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
