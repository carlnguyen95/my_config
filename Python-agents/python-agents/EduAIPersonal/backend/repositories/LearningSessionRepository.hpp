#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteLearningSessionRepository final : public LearningSessionRepository {
 public:
  explicit SqliteLearningSessionRepository(SqliteDatabase& db) : db_(db) {}

  std::optional<LearningSession> find_by_id(Id) override;
  LearningSession save(LearningSession) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
