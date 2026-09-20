#include "backend/repositories/Database.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <sqlite3.h>

namespace edu_ai::repositories {
namespace {
/**
 * @brief Builds a SQLite error message.
 * @param db Database handle.
 * @param prefix Context text.
 * @return Error message.
 */
std::string error_for(sqlite3* db, const std::string& prefix) {
  return prefix + ": " + (db == nullptr ? "SQLite handle unavailable" : sqlite3_errmsg(db));
}
}  // namespace

/**
 * @brief Opens SQLite and enables safety pragmas.
 * @param path Database file path.
 */
SqliteDatabase::SqliteDatabase(const std::filesystem::path& path) {
  std::filesystem::create_directories(path.parent_path().empty() ? "." : path.parent_path());
  if (sqlite3_open_v2(path.string().c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                      nullptr) != SQLITE_OK) {
    const auto message = error_for(db_, "Cannot open SQLite database");
    if (db_ != nullptr)
      sqlite3_close(db_);
    db_ = nullptr;
    throw std::runtime_error(message);
  }
  execute("PRAGMA foreign_keys = ON;");
  execute("PRAGMA journal_mode = WAL;");
}

/**
 * @brief Closes the owned SQLite connection.
 */
SqliteDatabase::~SqliteDatabase() {
  if (db_ != nullptr)
    sqlite3_close(db_);
}

/**
 * @brief Executes trusted SQL.
 * @param sql SQL statement.
 */
void SqliteDatabase::execute(const std::string& sql) const {
  char* error = nullptr;
  if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
    const std::string message =
        "SQLite statement failed: " + std::string(error == nullptr ? sqlite3_errmsg(db_) : error);
    sqlite3_free(error);
    throw std::runtime_error(message);
  }
}

/**
 * @brief Executes a schema file.
 * @param schema_path SQL schema path.
 */
void SqliteDatabase::initialize_schema(const std::filesystem::path& schema_path) {
  std::ifstream input(schema_path);
  if (!input)
    throw std::runtime_error("Cannot read schema file: " + schema_path.string());
  std::ostringstream contents;
  contents << input.rdbuf();
  execute(contents.str());
}

/**
 * @brief Formats current UTC time.
 * @return ISO-8601 timestamp.
 */
std::string utc_now() {
  const auto now = std::chrono::system_clock::now();
  const auto time = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  gmtime_s(&tm, &time);
#else
  gmtime_r(&time, &tm);
#endif
  std::ostringstream output;
  output << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
  return output.str();
}
}  // namespace edu_ai::repositories
