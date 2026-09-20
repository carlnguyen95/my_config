#include "backend/repositories/EnrollmentRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

/**
 * @brief Checks an enrollment.
 * @param user User ID.
 * @param course Course ID.
 * @return True when enrolled.
 */
bool SqliteEnrollmentRepository::exists(Id user, Id course) {
  auto s = prepare(db_, "SELECT 1 FROM enrollments WHERE user_id=? AND course_id=?;");
  bind(s.get(), 1, user);
  bind(s.get(), 2, course);
  return sqlite3_step(s.get()) == SQLITE_ROW;
}

/**
 * @brief Creates an enrollment.
 * @param user User ID.
 * @param course Course ID.
 * @return True when inserted.
 */
bool SqliteEnrollmentRepository::create(Id user, Id course) {
  auto s = prepare(db_, "INSERT OR IGNORE INTO enrollments(user_id,course_id,created_at) VALUES(?,?,?);");
  bind(s.get(), 1, user);
  bind(s.get(), 2, course);
  bind(s.get(), 3, utc_now());
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}

/**
 * @brief Deletes an enrollment.
 * @param user User ID.
 * @param course Course ID.
 * @return True when deleted.
 */
bool SqliteEnrollmentRepository::remove(Id user, Id course) {
  auto s = prepare(db_, "DELETE FROM enrollments WHERE user_id=? AND course_id=?;");
  bind(s.get(), 1, user);
  bind(s.get(), 2, course);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
