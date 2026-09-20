#include "backend/repositories/SubjectRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

/**
 * @brief Finds a subject.
 * @param id Subject ID.
 * @return Subject or no value.
 */
std::optional<Subject> SqliteSubjectRepository::find_by_id(Id id) {
  auto s = prepare(db_, "SELECT id,name,description FROM subjects WHERE id=?;");
  bind(s.get(), 1, id);
  if (sqlite3_step(s.get()) != SQLITE_ROW)
    return std::nullopt;
  return Subject{.id = sqlite3_column_int64(s.get(), 0), .name = text(s.get(), 1), .description = text(s.get(), 2)};
}

/**
 * @brief Saves a subject.
 * @param subject Subject fields.
 * @return Persisted subject.
 */
Subject SqliteSubjectRepository::save(Subject subject) {
  if (subject.id == 0) {
    auto s = prepare(db_, "INSERT INTO subjects(name,description) VALUES(?,?);");
    bind(s.get(), 1, subject.name);
    bind(s.get(), 2, subject.description);
    done(s.get());
    subject.id = sqlite3_last_insert_rowid(db_.handle());
  } else {
    auto s = prepare(db_, "UPDATE subjects SET name=?,description=? WHERE id=?;");
    bind(s.get(), 1, subject.name);
    bind(s.get(), 2, subject.description);
    bind(s.get(), 3, subject.id);
    done(s.get());
  }
  return subject;
}

/**
 * @brief Searches subjects by name.
 * @param name Search text.
 * @return Subjects.
 */
std::vector<Subject> SqliteSubjectRepository::find_by_name(const std::string& name) {
  auto statement = prepare(db_, "SELECT id,name,description FROM subjects WHERE name LIKE ? ORDER BY name;");
  bind(statement.get(), 1, "%" + name + "%");
  std::vector<Subject> result;
  while (sqlite3_step(statement.get()) == SQLITE_ROW) {
    result.push_back({.id = sqlite3_column_int64(statement.get(), 0),
                      .name = text(statement.get(), 1),
                      .description = text(statement.get(), 2)});
  }
  return result;
}

/**
 * @brief Deletes a subject.
 * @param id Subject ID.
 * @return True when deleted.
 */
bool SqliteSubjectRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM subjects WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
