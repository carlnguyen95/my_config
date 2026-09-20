#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteMessageRepository final : public MessageRepository {
 public:
  explicit SqliteMessageRepository(SqliteDatabase& db) : db_(db) {}

  std::vector<Message> recent_for_session(Id, int) override;
  std::vector<Message> find_by_content(Id, const std::string&) override;
  std::vector<Message> find_by_created_at(Id, const std::string&) override;
  Message save(Message) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
