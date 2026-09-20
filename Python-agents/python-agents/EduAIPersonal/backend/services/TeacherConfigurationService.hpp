#pragma once

#include "backend/repositories/Repositories.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::services {

class DefaultTeacherConfigurationService final : public TeacherConfigurationService {
 public:
  DefaultTeacherConfigurationService(repositories::TeacherConfigurationRepository& configurations,
                                     repositories::CourseRepository& courses, repositories::UserRepository& users)
      : configurations_(configurations), courses_(courses), users_(users) {}

  common::Result<TeacherConfiguration> get(Id actor_id, Id teacher_id, Id course_id) override;
  common::Result<TeacherConfiguration> save(Id actor_id, TeacherConfiguration configuration) override;

 private:
  repositories::TeacherConfigurationRepository& configurations_;
  repositories::CourseRepository& courses_;
  repositories::UserRepository& users_;
};

}  // namespace edu_ai::services
