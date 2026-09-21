#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#ifdef EDU_AI_ENABLE_DROGON
#include <drogon/drogon.h>
#include "backend/controllers/Router.hpp"
#endif

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

#ifdef EDU_AI_ENABLE_DROGON
  std::filesystem::path source_root = std::filesystem::path(EDU_AI_SOURCE_DIR);
  std::filesystem::path cert_path = source_root / "config/ssl/server.crt";
  std::filesystem::path key_path = source_root / "config/ssl/server.key";
  std::filesystem::path database_path = source_root / "data/edu_ai.db";
  int port = 8443;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--port" && i + 1 < argc) {
      port = std::stoi(argv[++i]);
    } else if (arg == "--cert" && i + 1 < argc) {
      cert_path = argv[++i];
    } else if (arg == "--key" && i + 1 < argc) {
      key_path = argv[++i];
    } else if (arg == "--database" && i + 1 < argc) {
      database_path = argv[++i];
    }
  }

  if (!std::filesystem::exists(cert_path) || !std::filesystem::exists(key_path)) {
    std::cerr << "Missing HTTPS certificate or key. Expected files:\n"
              << "  " << cert_path << "\n"
              << "  " << key_path << "\n"
              << "Generate a local self-signed pair with: openssl req -x509 -nodes -newkey rsa:2048 "
                 "-keyout config/ssl/server.key -out config/ssl/server.crt -days 365 -subj '/CN=localhost'\n";
    return 1;
  }

  const char* jwt_secret_env = std::getenv("EDU_AI_JWT_SECRET");
  if (jwt_secret_env == nullptr || std::string(jwt_secret_env).size() < 32) {
    std::cerr << "EDU_AI_JWT_SECRET must be set to a random value of at least 32 characters before starting HTTPS.\n";
    return 1;
  }

  drogon::app().setLogLevel(trantor::Logger::LogLevel::kWarn);
  drogon::app().setThreadNum(4);
  drogon::app().setSSLFiles(cert_path.string(), key_path.string());
  drogon::app().addListener("0.0.0.0", static_cast<uint16_t>(port), true, cert_path.string(), key_path.string());
  std::filesystem::create_directories(database_path.parent_path());
  edu_ai::controllers::configure_http_runtime(database_path, source_root / "database/init.sql", jwt_secret_env);

  std::cout << "Edu AI HTTPS server started on https://0.0.0.0:" << port << "\n";
  drogon::app().run();
  return 0;
#else
  std::cout << "Edu AI core is ready. Usage: edu_ai --init-db <database-path>\n";
  return 0;
#endif
}
