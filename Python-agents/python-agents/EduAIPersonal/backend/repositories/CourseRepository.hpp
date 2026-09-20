#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/repositories/Database.hpp"

namespace edu_ai::repositories {
class SqliteCourseRepository final : public CourseRepository {
 public:
  explicit SqliteCourseRepository(SqliteDatabase& db) : db_(db) {}

  std::optional<Course> find_by_id(Id) override;
  std::vector<Course> find_by_name(const std::string&) override;
  std::vector<Course> find_by_created_at(const std::string&) override;
  Course save(Course) override;
  bool is_teacher_for(Id, Id) override;
  bool remove(Id) override;

 private:
  SqliteDatabase& db_;
};
}  // namespace edu_ai::repositories
