#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteDocumentChunkRepository final : public DocumentChunkRepository {
 public:
  explicit SqliteDocumentChunkRepository(SqliteDatabase& db) : db_(db) {}

  std::vector<DocumentChunk> list_for_document(Id) override;
  std::vector<DocumentChunk> find_by_content(Id, const std::string&) override;
  DocumentChunk save(DocumentChunk) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
