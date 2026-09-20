#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteSubjectRepository final : public SubjectRepository {
 public:
  explicit SqliteSubjectRepository(SqliteDatabase& db) : db_(db) {}

  std::optional<Subject> find_by_id(Id) override;
  std::vector<Subject> find_by_name(const std::string&) override;
  Subject save(Subject) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
