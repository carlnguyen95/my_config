#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteRoadmapRepository final : public RoadmapRepository {
 public:
  explicit SqliteRoadmapRepository(SqliteDatabase& db) : db_(db) {}

  std::optional<Roadmap> find_by_id(Id) override;
  std::optional<Roadmap> find_for_user_course(Id, Id) override;
  Roadmap save(Roadmap) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
