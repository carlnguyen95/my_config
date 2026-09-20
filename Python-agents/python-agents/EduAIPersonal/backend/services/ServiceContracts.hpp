#pragma once

#include <string>

#include "backend/common/Result.hpp"
#include "backend/models/Domain.hpp"

namespace edu_ai::services {
using namespace edu_ai::models;

class AuthService {
 public:
  virtual ~AuthService() = default;
  /// Registers a non-admin account after validating the supplied credentials.
  virtual common::Result<User> register_user(const std::string& name, const std::string& email,
                                             const std::string& password, Role role) = 0;
  /// Authenticates an account and returns a signed access token.
  virtual common::Result<std::string> login(const std::string& email, const std::string& password) = 0;
};

class CourseService {
 public:
  virtual ~CourseService() = default;
  /// Returns a course only when the actor is allowed to access it.
  virtual common::Result<Course> get_course(Id actor_id, Id course_id) = 0;
  /// Creates a course while enforcing teacher ownership.
  virtual common::Result<Course> create_course(Id actor_id, Course course) = 0;
  /// Updates a course after verifying the actor manages it.
  virtual common::Result<Course> update_course(Id actor_id, Course course) = 0;
  /// Enrolls the authenticated student in a course.
  virtual common::Result<bool> enroll_self(Id actor_id, Id course_id) = 0;
  /// Removes the authenticated student's enrollment from a course.
  virtual common::Result<bool> unenroll_self(Id actor_id, Id course_id) = 0;
};

class TeacherConfigurationService {
 public:
  virtual ~TeacherConfigurationService() = default;
  /// Returns the policy configuration when the actor manages that course.
  virtual common::Result<TeacherConfiguration> get(Id actor_id, Id teacher_id, Id course_id) = 0;
  /// Creates or updates the course-scoped policy owned by its teacher.
  virtual common::Result<TeacherConfiguration> save(Id actor_id, TeacherConfiguration configuration) = 0;
};

class LearningService {
 public:
  virtual ~LearningService() = default;
};

class QuestionService {
 public:
  virtual ~QuestionService() = default;
  /// Returns a question visible to the actor under its publication rules.
  virtual common::Result<Question> find(Id actor_id, Id question_id) = 0;
  /// Creates a draft question for a course managed by the actor.
  virtual common::Result<Question> create(Id actor_id, Question question) = 0;
  /// Publishes a question after the actor's course ownership is verified.
  virtual common::Result<Question> approve(Id actor_id, Id question_id) = 0;
  /// Archives a question after the actor's course ownership is verified.
  virtual common::Result<bool> archive(Id actor_id, Id question_id) = 0;
  /// Permanently removes a question after the actor's ownership is verified.
  virtual common::Result<bool> remove(Id actor_id, Id question_id) = 0;
};

class RoadmapService {
 public:
  virtual ~RoadmapService() = default;
  /// Returns a roadmap to its owner or the teacher managing the course.
  virtual common::Result<Roadmap> get(Id actor_id, Id user_id, Id course_id) = 0;
  /// Saves a roadmap for a student in a teacher-managed course.
  virtual common::Result<Roadmap> save(Id actor_id, Roadmap roadmap) = 0;
  /// Updates an existing roadmap in a teacher-managed course.
  virtual common::Result<Roadmap> update(Id actor_id, Roadmap roadmap) = 0;
};

class ProgressService {
 public:
  virtual ~ProgressService() = default;
  /// Lists progress for its owner or the teacher managing the course.
  virtual common::Result<std::vector<LearningProgress>> list(Id actor_id, Id user_id, Id course_id) = 0;
  /// Updates progress after validating access and the numeric range.
  virtual common::Result<LearningProgress> update(Id actor_id, LearningProgress progress) = 0;
};

class AssessmentService {
 public:
  virtual ~AssessmentService() = default;
  /// Lists the authenticated student's assessments in a course they can access.
  virtual common::Result<std::vector<ThinkingAssessment>> list_for_self(Id actor_id, Id course_id) = 0;
  /// Lists assessments for a course managed by the actor.
  virtual common::Result<std::vector<ThinkingAssessment>> list_for_course(Id actor_id, Id course_id) = 0;
  /// Saves a teacher review after validating score and course ownership.
  virtual common::Result<ThinkingAssessment> review(Id actor_id, ThinkingAssessment assessment) = 0;
  /// Creates an assessment in a teacher-managed course.
  virtual common::Result<ThinkingAssessment> create(Id actor_id, ThinkingAssessment assessment) = 0;
};

}  // namespace edu_ai::services
