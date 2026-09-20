#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteQuestionRepository final : public QuestionRepository {
 public:
  explicit SqliteQuestionRepository(SqliteDatabase& db) : db_(db) {}

  std::vector<Question> search(Id, const std::string&, bool) override;
  std::optional<Question> find_by_id(Id) override;
  std::vector<Question> find_by_status(Id, QuestionStatus) override;
  std::vector<Question> find_by_content(Id, const std::string&) override;
  std::vector<Question> find_by_created_at(Id, const std::string&) override;
  Question save(Question) override;
  bool archive(Id) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
