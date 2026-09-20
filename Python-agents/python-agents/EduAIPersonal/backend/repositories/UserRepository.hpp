#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteUserRepository final : public UserRepository {
 public:
  explicit SqliteUserRepository(SqliteDatabase& db) : db_(db) {}

  std::optional<User> find_by_email(const std::string&) override;
  std::optional<User> find_by_id(Id) override;
  std::vector<User> find_by_name(const std::string&) override;
  std::vector<User> find_by_status(const std::string&) override;
  std::vector<User> find_by_created_at(const std::string&) override;
  User create(User) override;
  bool deactivate(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
