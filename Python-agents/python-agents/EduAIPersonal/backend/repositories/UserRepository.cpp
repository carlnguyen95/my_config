#include "backend/repositories/UserRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

/**
 * @brief Finds a user by email.
 * @param email Account email.
 * @return User or no value.
 */
std::optional<User> SqliteUserRepository::find_by_email(const std::string& email) {
  auto s = prepare(db_, "SELECT id,name,email,password_hash,role,status FROM users WHERE email=?;");
  bind(s.get(), 1, email);
  if (sqlite3_step(s.get()) != SQLITE_ROW)
    return std::nullopt;
  return User{.id = sqlite3_column_int64(s.get(), 0),
              .name = text(s.get(), 1),
              .email = text(s.get(), 2),
              .password_hash = text(s.get(), 3),
              .role = role_from(text(s.get(), 4)),
              .status = text(s.get(), 5)};
}

/**
 * @brief Finds a user.
 * @param id User ID.
 * @return User or no value.
 */
std::optional<User> SqliteUserRepository::find_by_id(Id id) {
  auto s = prepare(db_, "SELECT id,name,email,password_hash,role,status FROM users WHERE id=?;");
  bind(s.get(), 1, id);
  if (sqlite3_step(s.get()) != SQLITE_ROW)
    return std::nullopt;
  return User{.id = sqlite3_column_int64(s.get(), 0),
              .name = text(s.get(), 1),
              .email = text(s.get(), 2),
              .password_hash = text(s.get(), 3),
              .role = role_from(text(s.get(), 4)),
              .status = text(s.get(), 5)};
}

/**
 * @brief Creates a user.
 * @param user User fields.
 * @return Persisted user.
 */
User SqliteUserRepository::create(User user) {
  auto s = prepare(
      db_, "INSERT INTO users(name,email,password_hash,role,status,created_at,updated_at) VALUES(?,?,?,?,?,?,?);");
  const auto now = utc_now();
  bind(s.get(), 1, user.name);
  bind(s.get(), 2, user.email);
  bind(s.get(), 3, user.password_hash);
  bind(s.get(), 4, role_to(user.role));
  bind(s.get(), 5, user.status);
  bind(s.get(), 6, now);
  bind(s.get(), 7, now);
  done(s.get());
  user.id = sqlite3_last_insert_rowid(db_.handle());
  return user;
}

using namespace sqlite_detail;

/**
 * @brief Maps selected columns to a user.
 * @param statement SQLite row.
 * @return Domain user.
 */
static User user_row(sqlite3_stmt* statement) {
  return {.id = sqlite3_column_int64(statement, 0),
          .name = text(statement, 1),
          .email = text(statement, 2),
          .password_hash = text(statement, 3),
          .role = role_from(text(statement, 4)),
          .status = text(statement, 5)};
}

/**
 * @brief Searches users by name.
 * @param name Search text.
 * @return Users.
 */
std::vector<User> SqliteUserRepository::find_by_name(const std::string& name) {
  auto statement =
      prepare(db_, "SELECT id,name,email,password_hash,role,status FROM users WHERE name LIKE ? ORDER BY id;");
  bind(statement.get(), 1, "%" + name + "%");
  std::vector<User> result;
  while (sqlite3_step(statement.get()) == SQLITE_ROW) result.push_back(user_row(statement.get()));
  return result;
}

/**
 * @brief Filters users by status.
 * @param status Account status.
 * @return Users.
 */
std::vector<User> SqliteUserRepository::find_by_status(const std::string& status) {
  auto statement =
      prepare(db_, "SELECT id,name,email,password_hash,role,status FROM users WHERE status=? ORDER BY id;");
  bind(statement.get(), 1, status);
  std::vector<User> result;
  while (sqlite3_step(statement.get()) == SQLITE_ROW) result.push_back(user_row(statement.get()));
  return result;
}

/**
 * @brief Filters users by timestamp.
 * @param created_at UTC timestamp.
 * @return Users.
 */
std::vector<User> SqliteUserRepository::find_by_created_at(const std::string& created_at) {
  auto statement =
      prepare(db_, "SELECT id,name,email,password_hash,role,status FROM users WHERE created_at=? ORDER BY id;");
  bind(statement.get(), 1, created_at);
  std::vector<User> result;
  while (sqlite3_step(statement.get()) == SQLITE_ROW) result.push_back(user_row(statement.get()));
  return result;
}

/**
 * @brief Deactivates a user.
 * @param id User ID.
 * @return True when updated.
 */
bool SqliteUserRepository::deactivate(Id id) {
  auto s = prepare(db_, "UPDATE users SET status='inactive', updated_at=? WHERE id=?;");
  bind(s.get(), 1, utc_now());
  bind(s.get(), 2, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
