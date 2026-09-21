#include "backend/repositories/AssessmentRepository.hpp"
#include "backend/repositories/RepositorySupport.hpp"

namespace edu_ai::repositories {
using namespace sqlite_detail;

namespace {
/**
 * @brief Maps selected columns to an assessment.
 * @param s SQLite row.
 * @return Domain assessment.
 */
ThinkingAssessment row(sqlite3_stmt* s) {
  ThinkingAssessment a{.id = sqlite3_column_int64(s, 0),
                       .user_id = sqlite3_column_int64(s, 1),
                       .course_id = sqlite3_column_int64(s, 2),
                       .message_id = sqlite3_column_int64(s, 3),
                       .score = sqlite3_column_double(s, 4),
                       .dimensions_json = text(s, 5),
                       .reasoning = text(s, 6),
                       .model_name = text(s, 7),
                       .review_status = text(s, 10)};
  if (sqlite3_column_type(s, 8) != SQLITE_NULL)
    a.teacher_score = sqlite3_column_double(s, 8);
  if (sqlite3_column_type(s, 9) != SQLITE_NULL)
    a.teacher_feedback = text(s, 9);
  return a;
}

constexpr auto kColumns =
    "id,user_id,course_id,message_id,score,dimensions_json,reasoning,model_name,teacher_score,teacher_feedback,review_"
    "status";
}  // namespace

/**
 * @brief Finds an assessment.
 * @param id Assessment ID.
 * @return Assessment or no value.
 */
std::optional<ThinkingAssessment> SqliteAssessmentRepository::find_by_id(Id id) {
  auto s = prepare(db_,
                   "SELECT "
                   "id,user_id,course_id,message_id,score,dimensions_json,reasoning,model_name,teacher_score,teacher_"
                   "feedback,review_status FROM thinking_assessments WHERE id=?;");
  bind(s.get(), 1, id);
  return sqlite3_step(s.get()) == SQLITE_ROW ? std::optional<ThinkingAssessment>{row(s.get())} : std::nullopt;
}

/**
 * @brief Lists a student's assessments in a course.
 * @param user_id Student ID.
 * @param course_id Course ID.
 * @return Assessments in newest-first order.
 */
std::vector<ThinkingAssessment> SqliteAssessmentRepository::list_for_user_course(Id user_id, Id course_id) {
  auto s = prepare(db_,
                   "SELECT "
                   "id,user_id,course_id,message_id,score,dimensions_json,reasoning,model_name,teacher_score,teacher_"
                   "feedback,review_status FROM thinking_assessments WHERE user_id=? AND course_id=? ORDER BY id DESC;");
  bind(s.get(), 1, user_id);
  bind(s.get(), 2, course_id);
  std::vector<ThinkingAssessment> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(row(s.get()));
  return result;
}

/**
 * @brief Lists course assessments.
 * @param course Course ID.
 * @return Assessments.
 */
std::vector<ThinkingAssessment> SqliteAssessmentRepository::list_for_course(Id course) {
  auto s = prepare(db_,
                   "SELECT "
                   "id,user_id,course_id,message_id,score,dimensions_json,reasoning,model_name,teacher_score,teacher_"
                   "feedback,review_status FROM thinking_assessments WHERE course_id=? ORDER BY id DESC;");
  bind(s.get(), 1, course);
  std::vector<ThinkingAssessment> r;
  while (sqlite3_step(s.get()) == SQLITE_ROW) r.push_back(row(s.get()));
  return r;
}

/**
 * @brief Searches course assessments for a text query.
 * @param course_id Course ID.
 * @param query Search text.
 * @return Matching assessments.
 */
std::vector<ThinkingAssessment> SqliteAssessmentRepository::find_in_course(Id course_id, const std::string& query) {
  auto s = prepare(db_,
                   "SELECT "
                   "id,user_id,course_id,message_id,score,dimensions_json,reasoning,model_name,teacher_score,teacher_"
                   "feedback,review_status FROM thinking_assessments WHERE course_id=? AND (reasoning LIKE ? OR "
                   "dimensions_json LIKE ? OR model_name LIKE ?) ORDER BY id DESC;");
  bind(s.get(), 1, course_id);
  bind(s.get(), 2, "%" + query + "%");
  bind(s.get(), 3, "%" + query + "%");
  bind(s.get(), 4, "%" + query + "%");
  std::vector<ThinkingAssessment> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(row(s.get()));
  return result;
}

/**
 * @brief Saves an assessment.
 * @param a Assessment fields.
 * @return Persisted assessment.
 */
ThinkingAssessment SqliteAssessmentRepository::save(ThinkingAssessment a) {
  if (a.id == 0) {
    auto s = prepare(db_,
                     "INSERT INTO "
                     "thinking_assessments(user_id,course_id,message_id,score,dimensions_json,reasoning,model_name,"
                     "teacher_score,teacher_feedback,review_status,created_at) VALUES(?,?,?,?,?,?,?,?,?,?,?);");
    bind(s.get(), 1, a.user_id);
    bind(s.get(), 2, a.course_id);
    bind(s.get(), 3, a.message_id);
    bind(s.get(), 4, a.score);
    bind(s.get(), 5, a.dimensions_json);
    bind(s.get(), 6, a.reasoning);
    bind(s.get(), 7, a.model_name);
    if (a.teacher_score)
      bind(s.get(), 8, *a.teacher_score);
    else
      bind_null(s.get(), 8);
    if (a.teacher_feedback)
      bind(s.get(), 9, *a.teacher_feedback);
    else
      bind_null(s.get(), 9);
    bind(s.get(), 10, a.review_status);
    bind(s.get(), 11, utc_now());
    done(s.get());
    a.id = sqlite3_last_insert_rowid(db_.handle());
  } else {
    auto s =
        prepare(db_, "UPDATE thinking_assessments SET teacher_score=?,teacher_feedback=?,review_status=? WHERE id=?;");
    if (a.teacher_score)
      bind(s.get(), 1, *a.teacher_score);
    else
      bind_null(s.get(), 1);
    if (a.teacher_feedback)
      bind(s.get(), 2, *a.teacher_feedback);
    else
      bind_null(s.get(), 2);
    bind(s.get(), 3, a.review_status);
    bind(s.get(), 4, a.id);
    done(s.get());
  }
  return a;
}

using namespace sqlite_detail;

/**
 * @brief Filters assessments by review status.
 * @param course_id Course ID.
 * @param review_status Status.
 * @return Assessments.
 */
std::vector<ThinkingAssessment> SqliteAssessmentRepository::find_by_status(Id course_id,
                                                                           const std::string& review_status) {
  auto s = prepare(
      db_,
      "SELECT "
      "id,user_id,course_id,message_id,score,dimensions_json,reasoning,model_name,teacher_score,teacher_feedback,"
      "review_status FROM thinking_assessments WHERE course_id=? AND review_status=? ORDER BY id DESC;");
  bind(s.get(), 1, course_id);
  bind(s.get(), 2, review_status);
  std::vector<ThinkingAssessment> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(row(s.get()));
  return result;
}

/**
 * @brief Filters assessments by timestamp.
 * @param course_id Course ID.
 * @param created_at UTC timestamp.
 * @return Assessments.
 */
std::vector<ThinkingAssessment> SqliteAssessmentRepository::find_by_created_at(Id course_id,
                                                                               const std::string& created_at) {
  auto s =
      prepare(db_,
              "SELECT "
              "id,user_id,course_id,message_id,score,dimensions_json,reasoning,model_name,teacher_score,teacher_"
              "feedback,review_status FROM thinking_assessments WHERE course_id=? AND created_at=? ORDER BY id DESC;");
  bind(s.get(), 1, course_id);
  bind(s.get(), 2, created_at);
  std::vector<ThinkingAssessment> result;
  while (sqlite3_step(s.get()) == SQLITE_ROW) result.push_back(row(s.get()));
  return result;
}

/**
 * @brief Deletes an assessment.
 * @param id Assessment ID.
 * @return True when deleted.
 */
bool SqliteAssessmentRepository::remove(Id id) {
  auto s = prepare(db_, "DELETE FROM thinking_assessments WHERE id=?;");
  bind(s.get(), 1, id);
  done(s.get());
  return sqlite3_changes(db_.handle()) == 1;
}
}  // namespace edu_ai::repositories
