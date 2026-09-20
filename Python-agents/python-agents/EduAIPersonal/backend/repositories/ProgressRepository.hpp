#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteProgressRepository final : public ProgressRepository {
 public:
  explicit SqliteProgressRepository(SqliteDatabase& db) : db_(db) {}

  std::vector<LearningProgress> list(Id, Id) override;
  LearningProgress save(LearningProgress) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
