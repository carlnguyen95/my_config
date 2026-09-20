#pragma once

#include "backend/repositories/Database.hpp"
#include "backend/repositories/Repositories.hpp"

namespace edu_ai::repositories {

class SqliteTeacherConfigurationRepository final : public TeacherConfigurationRepository {
 public:
  explicit SqliteTeacherConfigurationRepository(SqliteDatabase& db) : db_(db) {}

  std::optional<TeacherConfiguration> find_for_teacher_course(Id teacher_id, Id course_id) override;
  TeacherConfiguration save(TeacherConfiguration configuration) override;
  bool remove(Id id) override;

 private:
  SqliteDatabase& db_;
};

}  // namespace edu_ai::repositories
