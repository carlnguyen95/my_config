#pragma once

#include <filesystem>
#include <string>

#ifdef EDU_AI_ENABLE_DROGON
#include <drogon/drogon.h>
#endif

#include "backend/bootstrap/HttpRuntime.hpp"
#include "backend/controllers/Controllers.hpp"

namespace edu_ai::controllers {

#ifdef EDU_AI_ENABLE_DROGON
/// Initializes application dependencies before Drogon auto-creates HTTP controllers.
inline void configure_http_runtime(const std::filesystem::path& database_path,
                                   const std::filesystem::path& schema_path, std::string jwt_secret) {
  bootstrap::HttpRuntime::initialize(database_path, schema_path, std::move(jwt_secret));

  // Cross-origin support is intentionally limited to the local Vite development server.
  drogon::app().registerPreSendingAdvice([](const drogon::HttpRequestPtr&, const drogon::HttpResponsePtr& response) {
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
    response->addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PATCH, OPTIONS");
  });

  // Health is infrastructure-level rather than a domain controller route.
  drogon::app().registerHandler("/api/health", [](const drogon::HttpRequestPtr&,
                                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    Json::Value payload(Json::objectValue);
    payload["status"] = "ok";
    payload["transport"] = "https";
    callback(drogon::HttpResponse::newHttpJsonResponse(payload));
  }, {drogon::Get});
}
#endif

}  // namespace edu_ai::controllers
