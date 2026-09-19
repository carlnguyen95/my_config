#pragma once

#include <string>

#include "backend/common/Result.hpp"
#include "backend/models/Domain.hpp"

namespace edu_ai::services {
using namespace edu_ai::models;

class AuthService { public: virtual ~AuthService() = default; virtual common::Result<User> register_user(const std::string& name, const std::string& email, const std::string& password, Role role) = 0; virtual common::Result<std::string> login(const std::string& email, const std::string& password) = 0; };
class CourseService { public: virtual ~CourseService() = default; virtual common::Result<Course> get_course(Id actor_id, Id course_id) = 0; };
class LearningService { public: virtual ~LearningService() = default; };
class QuestionService { public: virtual ~QuestionService() = default; virtual common::Result<Question> create(Id actor_id, Question question) = 0; };
class RoadmapService { public: virtual ~RoadmapService() = default; };
class ProgressService { public: virtual ~ProgressService() = default; };
class AssessmentService { public: virtual ~AssessmentService() = default; };

}  // namespace edu_ai::services

