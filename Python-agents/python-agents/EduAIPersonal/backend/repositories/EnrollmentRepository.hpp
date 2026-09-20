#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteEnrollmentRepository final : public EnrollmentRepository {
 public:
  explicit SqliteEnrollmentRepository(SqliteDatabase& db) : db_(db) {}

  bool exists(Id, Id) override;
  bool create(Id, Id) override;
  bool remove(Id, Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
