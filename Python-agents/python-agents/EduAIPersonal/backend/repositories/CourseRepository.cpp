#include "backend/repositories/CourseRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

/**
 * @brief Finds a course.
 * @param id Course ID.
 * @return Course or no value.
 */
std::optional<Course> SqliteCourseRepository::find_by_id(Id id) {
  auto s = prepare(db_, "SELECT id,subject_id,name,description,teacher_id,answer_policy FROM courses WHERE id=?;");
  bind(s.get(), 1, id);
  if (sqlite3_step(s.get()) != SQLITE_ROW)
    return std::nullopt;
  return Course{.id = sqlite3_column_int64(s.get(), 0),
                .subject_id = sqlite3_column_int64(s.get(), 1),
                .name = text(s.get(), 2),
                .description = text(s.get(), 3),
                .teacher_id = sqlite3_column_int64(s.get(), 4),
                .answer_policy = policy_from(text(s.get(), 5))};
}

/**
 * @brief Saves a course.
 * @param course Course fields.
 * @return Persisted course.
 */
Course SqliteCourseRepository::save(Course course) {
  const auto now = utc_now();
  if (course.id == 0) {
    auto s = prepare(db_,
                     "INSERT INTO courses(subject_id,name,description,teacher_id,answer_policy,created_at,updated_at) "
                     "VALUES(?,?,?,?,?,?,?);");
    bind(s.get(), 1, course.subject_id);
    bind(s.get(), 2, course.name);
    bind(s.get(), 3, course.description);
    bind(s.get(), 4, course.teacher_id);
    bind(s.get(), 5, policy_to(course.answer_policy));
    bind(s.get(), 6, now);
    bind(s.get(), 7, now);
    done(s.get());
    course.id = sqlite3_last_insert_rowid(db_.handle());
  } else {
    auto s = prepare(
        db_,
        "UPDATE courses SET subject_id=?,name=?,description=?,teacher_id=?,answer_policy=?,updated_at=? WHERE id=?;");
    bind(s.get(), 1, course.subject_id);
    bind(s.get(), 2, course.name);
    bind(s.get(), 3, course.description);
    bind(s.get(), 4, course.teacher_id);
    bind(s.get(), 5, policy_to(course.answer_policy));
    bind(s.get(), 6, now);
    bind(s.get(), 7, course.id);
    done(s.get());
  }
  return course;
}

/**
 * @brief Checks course ownership.
 * @param user User ID.
 * @param course Course ID.
 * @return True when owner.
 */
bool SqliteCourseRepository::is_teacher_for(Id user, Id course) {
  auto s = prepare(db_, "SELECT 1 FROM courses WHERE id=? AND teacher_id=?;");
  bind(s.get(), 1, course);
  bind(s.get(), 2, user);
  return sqlite3_step(s.get()) == SQLITE_ROW;
}

using namespace sqlite_detail;

namespace {
/**
 * @brief Maps selected columns to a course.
 * @param s SQLite row.
 * @return Domain course.
 */
Course course_row(sqlite3_stmt* s) {
  return {.id = sqlite3_column_int64(s, 0),
          .subject_id = sqlite3_column_int64(s, 1),
          .name = text(s, 2),
          .description = text(s, 3),
          .teacher_id = sqlite3_column_int64(s, 4),
          .answer_policy = policy_from(text(s, 5))};
}
}  // namespace

/**
 * @brief Searches courses by name.
 * @param name Search text.
 * @return Matching courses.
 */
std::vector<Course> SqliteCourseRepository::find_by_name(const std::string& name) {
  auto s = prepare(
      db_,
      "SELECT id,subject_id,name,description,teacher_id,answer_policy FROM courses WHERE name LIKE ? ORDER BY name;");
  bind(s.get(), 1, "%" + name + "%");
  std::vector<Course> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(course_row(s.get()));
  return result;
}

/**
 * @brief Filters courses by timestamp.
 * @param created_at UTC timestamp.
 * @return Matching courses.
 */
std::vector<Course> SqliteCourseRepository::find_by_created_at(const std::string& created_at) {
  auto s = prepare(
      db_,
      "SELECT id,subject_id,name,description,teacher_id,answer_policy FROM courses WHERE created_at=? ORDER BY id;");
  bind(s.get(), 1, created_at);
  std::vector<Course> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(course_row(s.get()));
  return result;
}

/**
 * @brief Deletes a course.
 * @param id Course ID.
 * @return True when deleted.
 */
bool SqliteCourseRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM courses WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
