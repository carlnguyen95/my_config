#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::services {
class DefaultAssessmentService final : public AssessmentService {
 public:
  DefaultAssessmentService(repositories::AssessmentRepository& assessments, repositories::CourseRepository& courses,
                           repositories::EnrollmentRepository& enrollments, repositories::UserRepository& users)
      : assessments_(assessments), courses_(courses), enrollments_(enrollments), users_(users) {}

  common::Result<std::vector<ThinkingAssessment>> list_for_self(Id, Id) override;
  common::Result<std::vector<ThinkingAssessment>> list_for_course(Id, Id) override;
  common::Result<std::vector<ThinkingAssessment>> find_in_course(Id, Id, const std::string&) override;
  common::Result<ThinkingAssessment> review(Id, ThinkingAssessment) override;
  common::Result<ThinkingAssessment> create(Id, ThinkingAssessment) override;

 private:
  repositories::AssessmentRepository& assessments_;
  repositories::CourseRepository& courses_;
  repositories::EnrollmentRepository& enrollments_;
  repositories::UserRepository& users_;
};
}  // namespace edu_ai::services
