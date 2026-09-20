#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteAssessmentRepository final : public AssessmentRepository {
 public:
  explicit SqliteAssessmentRepository(SqliteDatabase& db) : db_(db) {}

  std::optional<ThinkingAssessment> find_by_id(Id) override;
  std::vector<ThinkingAssessment> list_for_user_course(Id, Id) override;
  std::vector<ThinkingAssessment> list_for_course(Id) override;
  std::vector<ThinkingAssessment> find_by_status(Id, const std::string&) override;
  std::vector<ThinkingAssessment> find_by_created_at(Id, const std::string&) override;
  ThinkingAssessment save(ThinkingAssessment) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
