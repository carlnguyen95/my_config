#include "backend/repositories/TeacherConfigurationRepository.hpp"

#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

/**
 * @brief Finds teacher policy.
 * @param teacher_id Teacher ID.
 * @param course_id Course ID.
 * @return Configuration or no value.
 */
std::optional<TeacherConfiguration> SqliteTeacherConfigurationRepository::find_for_teacher_course(Id teacher_id,
                                                                                                  Id course_id) {
  auto statement = prepare(
      db_,
      "SELECT id,teacher_id,course_id,policy_text FROM teacher_configurations WHERE teacher_id=? AND course_id=?;");
  bind(statement.get(), 1, teacher_id);
  bind(statement.get(), 2, course_id);
  if (sqlite3_step(statement.get()) != SQLITE_ROW)
    return std::nullopt;
  return TeacherConfiguration{.id = sqlite3_column_int64(statement.get(), 0),
                              .teacher_id = sqlite3_column_int64(statement.get(), 1),
                              .course_id = sqlite3_column_int64(statement.get(), 2),
                              .policy_text = text(statement.get(), 3)};
}

/**
 * @brief Saves teacher policy.
 * @param configuration Policy fields.
 * @return Persisted configuration.
 */
TeacherConfiguration SqliteTeacherConfigurationRepository::save(TeacherConfiguration configuration) {
  const auto now = utc_now();
  if (configuration.id == 0) {
    auto statement =
        prepare(db_,
                "INSERT INTO teacher_configurations(teacher_id,course_id,policy_text,created_at,updated_at) "
                "VALUES(?,?,?,?,?);");
    bind(statement.get(), 1, configuration.teacher_id);
    bind(statement.get(), 2, configuration.course_id);
    bind(statement.get(), 3, configuration.policy_text);
    bind(statement.get(), 4, now);
    bind(statement.get(), 5, now);
    done(statement.get());
    configuration.id = sqlite3_last_insert_rowid(db_.handle());
  } else {
    auto statement = prepare(db_, "UPDATE teacher_configurations SET policy_text=?,updated_at=? WHERE id=?;");
    bind(statement.get(), 1, configuration.policy_text);
    bind(statement.get(), 2, now);
    bind(statement.get(), 3, configuration.id);
    done(statement.get());
  }
  return configuration;
}

/**
 * @brief Deletes teacher policy.
 * @param id Configuration ID.
 * @return True when deleted.
 */
bool SqliteTeacherConfigurationRepository::remove(Id id) {
  auto statement = prepare(db_, "DELETE FROM teacher_configurations WHERE id=?;");
  bind(statement.get(), 1, id);
  done(statement.get());
  return sqlite3_changes(db_.handle()) == 1;
}

}  // namespace edu_ai::repositories
