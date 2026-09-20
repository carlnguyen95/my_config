#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteDocumentRepository final : public DocumentRepository {
 public:
  explicit SqliteDocumentRepository(SqliteDatabase& db) : db_(db) {}

  Document save(Document) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
