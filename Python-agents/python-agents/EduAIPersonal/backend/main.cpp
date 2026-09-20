#include <filesystem>
#include <iostream>
#include <string>

#include "backend/repositories/Database.hpp"

/**
 * @brief Opens an optional SQLite database and initializes its schema for local use.
 * @param argc Number of command-line arguments.
 * @param argv Command-line argument values.
 * @return Process exit code.
 */
int main(int argc, char* argv[]) {
  if (argc == 3 && std::string(argv[1]) == "--init-db") {
    edu_ai::repositories::SqliteDatabase database(argv[2]);
    database.initialize_schema(std::filesystem::path(EDU_AI_SOURCE_DIR) / "database/init.sql");
    std::cout << "SQLite database initialized: " << argv[2] << "\n";
    return 0;
  }
  std::cout << "Edu AI core is ready. Usage: edu_ai --init-db <database-path>\n";
  return 0;
}
