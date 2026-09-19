#pragma once

#include <optional>
#include <string>
#include <vector>

#include "backend/models/Domain.hpp"

namespace edu_ai::repositories {
using namespace edu_ai::models;

class UserRepository { public: virtual ~UserRepository() = default; virtual std::optional<User> find_by_email(const std::string& email) = 0; virtual std::optional<User> find_by_id(Id id) = 0; virtual User create(User user) = 0; };
class CourseRepository { public: virtual ~CourseRepository() = default; virtual std::optional<Course> find_by_id(Id id) = 0; virtual bool is_enrolled(Id user_id, Id course_id) = 0; virtual bool is_teacher_for(Id user_id, Id course_id) = 0; };
class QuestionRepository { public: virtual ~QuestionRepository() = default; virtual std::vector<Question> search(Id course_id, const std::string& query, bool approved_only) = 0; virtual std::optional<Question> find_by_id(Id id) = 0; virtual Question save(Question question) = 0; };
class ConversationRepository { public: virtual ~ConversationRepository() = default; virtual std::vector<Message> recent_messages(Id session_id, int limit) = 0; virtual Message save_message(Message message) = 0; };
class RoadmapRepository { public: virtual ~RoadmapRepository() = default; virtual std::optional<Roadmap> find_for_user_course(Id user_id, Id course_id) = 0; virtual Roadmap save(Roadmap roadmap) = 0; };
class ProgressRepository { public: virtual ~ProgressRepository() = default; virtual std::vector<LearningProgress> list(Id user_id, Id course_id) = 0; virtual LearningProgress save(LearningProgress progress) = 0; };
class AssessmentRepository { public: virtual ~AssessmentRepository() = default; virtual std::optional<ThinkingAssessment> find_by_id(Id id) = 0; virtual std::vector<ThinkingAssessment> list_for_course(Id course_id) = 0; virtual ThinkingAssessment save(ThinkingAssessment assessment) = 0; };

}  // namespace edu_ai::repositories

