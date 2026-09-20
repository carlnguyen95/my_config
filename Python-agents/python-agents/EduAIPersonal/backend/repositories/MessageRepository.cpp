#include "backend/repositories/MessageRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

namespace {
/**
 * @brief Maps selected columns to a message.
 * @param s SQLite row.
 * @return Domain message.
 */
Message row(sqlite3_stmt* s) {
  Message m{.id = sqlite3_column_int64(s, 0),
            .session_id = sqlite3_column_int64(s, 1),
            .role = text(s, 2),
            .content = text(s, 3),
            .model = text(s, 6)};
  if (sqlite3_column_type(s, 4) != SQLITE_NULL)
    m.token_input = sqlite3_column_int(s, 4);
  if (sqlite3_column_type(s, 5) != SQLITE_NULL)
    m.token_output = sqlite3_column_int(s, 5);
  return m;
}
}  // namespace

/**
 * @brief Lists recent messages.
 * @param session Session ID.
 * @param limit Max results.
 * @return Messages.
 */
std::vector<Message> SqliteMessageRepository::recent_for_session(Id session, int limit) {
  auto s = prepare(db_,
                   "SELECT id,session_id,role,content,token_input,token_output,model FROM messages WHERE session_id=? "
                   "ORDER BY id DESC LIMIT ?;");
  bind(s.get(), 1, session);
  bind(s.get(), 2, limit);
  std::vector<Message> r;
  while (sqlite3_step(s.get()) == SQLITE_ROW) r.push_back(row(s.get()));
  return r;
}

/**
 * @brief Saves a message.
 * @param message Message fields.
 * @return Persisted message.
 */
Message SqliteMessageRepository::save(Message message) {
  auto s = prepare(
      db_,
      "INSERT INTO messages(session_id,role,content,token_input,token_output,model,created_at) VALUES(?,?,?,?,?,?,?);");
  bind(s.get(), 1, message.session_id);
  bind(s.get(), 2, message.role);
  bind(s.get(), 3, message.content);
  if (message.token_input)
    bind(s.get(), 4, *message.token_input);
  else
    bind_null(s.get(), 4);
  if (message.token_output)
    bind(s.get(), 5, *message.token_output);
  else
    bind_null(s.get(), 5);
  bind(s.get(), 6, message.model);
  bind(s.get(), 7, utc_now());
  done(s.get());
  message.id = sqlite3_last_insert_rowid(db_.handle());
  return message;
}

using namespace sqlite_detail;

/**
 * @brief Searches messages.
 * @param session_id Session ID.
 * @param content Search text.
 * @return Messages.
 */
std::vector<Message> SqliteMessageRepository::find_by_content(Id session_id, const std::string& content) {
  auto s = prepare(db_,
                   "SELECT id,session_id,role,content,token_input,token_output,model FROM messages WHERE session_id=? "
                   "AND content LIKE ? ORDER BY id;");
  bind(s.get(), 1, session_id);
  bind(s.get(), 2, "%" + content + "%");
  std::vector<Message> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(row(s.get()));
  return result;
}

/**
 * @brief Filters messages by timestamp.
 * @param session_id Session ID.
 * @param created_at UTC timestamp.
 * @return Messages.
 */
std::vector<Message> SqliteMessageRepository::find_by_created_at(Id session_id, const std::string& created_at) {
  auto s = prepare(db_,
                   "SELECT id,session_id,role,content,token_input,token_output,model FROM messages WHERE session_id=? "
                   "AND created_at=? ORDER BY id;");
  bind(s.get(), 1, session_id);
  bind(s.get(), 2, created_at);
  std::vector<Message> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(row(s.get()));
  return result;
}

/**
 * @brief Deletes a message.
 * @param id Message ID.
 * @return True when deleted.
 */
bool SqliteMessageRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM messages WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
