#pragma once
#include "backend/repositories/Repositories.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::services {
class DefaultRoadmapService final : public RoadmapService {
 public:
  DefaultRoadmapService(repositories::RoadmapRepository& roadmaps, repositories::CourseRepository& courses,
                        repositories::EnrollmentRepository& enrollments, repositories::UserRepository& users)
      : roadmaps_(roadmaps), courses_(courses), enrollments_(enrollments), users_(users) {}

  common::Result<Roadmap> get(Id, Id, Id) override;
  common::Result<Roadmap> save(Id, Roadmap) override;
  common::Result<Roadmap> update(Id, Roadmap) override;

 private:
  repositories::RoadmapRepository& roadmaps_;
  repositories::CourseRepository& courses_;
  repositories::EnrollmentRepository& enrollments_;
  repositories::UserRepository& users_;
};
}  // namespace edu_ai::services
