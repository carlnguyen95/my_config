#include "backend/repositories/DocumentChunkRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

/**
 * @brief Lists document chunks.
 * @param document Document ID.
 * @return Ordered chunks.
 */
std::vector<DocumentChunk> SqliteDocumentChunkRepository::list_for_document(Id document) {
  auto s = prepare(db_,
                   "SELECT id,document_id,chunk_index,content,COALESCE(embedding,'') FROM document_chunks WHERE "
                   "document_id=? ORDER BY chunk_index;");
  bind(s.get(), 1, document);
  std::vector<DocumentChunk> r;
  while (sqlite3_step(s.get()) == SQLITE_ROW)
    r.push_back({.id = sqlite3_column_int64(s.get(), 0),
                 .document_id = sqlite3_column_int64(s.get(), 1),
                 .chunk_index = sqlite3_column_int(s.get(), 2),
                 .content = text(s.get(), 3),
                 .embedding = text(s.get(), 4)});
  return r;
}

/**
 * @brief Saves a document chunk.
 * @param c Chunk fields.
 * @return Persisted chunk.
 */
DocumentChunk SqliteDocumentChunkRepository::save(DocumentChunk c) {
  auto s =
      prepare(db_,
              "INSERT INTO document_chunks(document_id,chunk_index,content,embedding) VALUES(?,?,?,?) ON "
              "CONFLICT(document_id,chunk_index) DO UPDATE SET content=excluded.content,embedding=excluded.embedding;");
  bind(s.get(), 1, c.document_id);
  bind(s.get(), 2, c.chunk_index);
  bind(s.get(), 3, c.content);
  bind(s.get(), 4, c.embedding);
  done(s.get());
  auto f = prepare(db_, "SELECT id FROM document_chunks WHERE document_id=? AND chunk_index=?;");
  bind(f.get(), 1, c.document_id);
  bind(f.get(), 2, c.chunk_index);
  if (sqlite3_step(f.get()) == SQLITE_ROW)
    c.id = sqlite3_column_int64(f.get(), 0);
  return c;
}

using namespace sqlite_detail;

/**
 * @brief Searches document chunks.
 * @param document_id Document ID.
 * @param content Search text.
 * @return Matching chunks.
 */
std::vector<DocumentChunk> SqliteDocumentChunkRepository::find_by_content(Id document_id, const std::string& content) {
  auto s = prepare(db_,
                   "SELECT id,document_id,chunk_index,content,COALESCE(embedding,'') FROM document_chunks WHERE "
                   "document_id=? AND content LIKE ? ORDER BY chunk_index;");
  bind(s.get(), 1, document_id);
  bind(s.get(), 2, "%" + content + "%");
  std::vector<DocumentChunk> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW)
    result.push_back({.id = sqlite3_column_int64(s.get(), 0),
                      .document_id = sqlite3_column_int64(s.get(), 1),
                      .chunk_index = sqlite3_column_int(s.get(), 2),
                      .content = text(s.get(), 3),
                      .embedding = text(s.get(), 4)});
  return result;
}

/**
 * @brief Deletes a document chunk.
 * @param id Chunk ID.
 * @return True when deleted.
 */
bool SqliteDocumentChunkRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM document_chunks WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
