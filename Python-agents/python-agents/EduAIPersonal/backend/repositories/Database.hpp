#pragma once

#include <filesystem>
#include <string>

struct sqlite3;

namespace edu_ai::repositories {

class SqliteDatabase {
 public:
  /// Opens a SQLite database and enables its required safety pragmas.
  explicit SqliteDatabase(const std::filesystem::path& path);
  /// Closes the owned SQLite connection.
  ~SqliteDatabase();
  SqliteDatabase(const SqliteDatabase&) = delete;
  SqliteDatabase& operator=(const SqliteDatabase&) = delete;

  /// Executes the database schema from a SQL file.
  void initialize_schema(const std::filesystem::path& schema_path);
  /// Executes a trusted SQL statement.
  void execute(const std::string& sql) const;

  sqlite3* handle() const {
    return db_;
  }

 private:
  sqlite3* db_{nullptr};
};

/// Returns the current UTC time serialized in ISO-8601 format.
std::string utc_now();

}  // namespace edu_ai::repositories
