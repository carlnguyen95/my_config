#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::services {
class DefaultQuestionService final : public QuestionService {
 public:
  DefaultQuestionService(repositories::QuestionRepository& questions, repositories::CourseRepository& courses,
                         repositories::EnrollmentRepository& enrollments, repositories::UserRepository& users)
      : questions_(questions), courses_(courses), enrollments_(enrollments), users_(users) {}

  common::Result<Question> find(Id, Id) override;
  common::Result<Question> create(Id, Question) override;
  common::Result<Question> approve(Id, Id) override;
  common::Result<bool> archive(Id, Id) override;
  common::Result<bool> remove(Id, Id) override;

 private:
  repositories::QuestionRepository& questions_;
  repositories::CourseRepository& courses_;
  repositories::EnrollmentRepository& enrollments_;
  repositories::UserRepository& users_;
};
}  // namespace edu_ai::services
