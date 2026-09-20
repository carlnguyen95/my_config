#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::services {
class DefaultProgressService final : public ProgressService {
 public:
  DefaultProgressService(repositories::ProgressRepository& progress, repositories::CourseRepository& courses,
                         repositories::EnrollmentRepository& enrollments, repositories::UserRepository& users)
      : progress_(progress), courses_(courses), enrollments_(enrollments), users_(users) {}

  common::Result<std::vector<LearningProgress>> list(Id, Id, Id) override;
  common::Result<LearningProgress> update(Id, LearningProgress) override;

 private:
  repositories::ProgressRepository& progress_;
  repositories::CourseRepository& courses_;
  repositories::EnrollmentRepository& enrollments_;
  repositories::UserRepository& users_;
};
}  // namespace edu_ai::services
