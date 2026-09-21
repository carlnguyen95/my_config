#pragma once
#include <optional>
#include <string>
#include <vector>
#include "backend/models/Domain.hpp"

namespace edu_ai::repositories {
using namespace edu_ai::models;

class UserRepository {
 public:
  virtual ~UserRepository() = default;
  /// Finds one user by unique email address.
  virtual std::optional<User> find_by_email(const std::string& email) = 0;
  /// Finds one user by primary key.
  virtual std::optional<User> find_by_id(Id id) = 0;
  /// Lists users whose names contain the supplied text.
  virtual std::vector<User> find_by_name(const std::string& name) = 0;
  /// Lists users with the requested account status.
  virtual std::vector<User> find_by_status(const std::string& status) = 0;
  /// Lists users created at the supplied UTC timestamp.
  virtual std::vector<User> find_by_created_at(const std::string& created_at) = 0;
  /// Marks an account inactive without deleting its history.
  virtual bool deactivate(Id id) = 0;
  /// Creates a new user and returns it with its generated ID.
  virtual User create(User user) = 0;
};

class SubjectRepository {
 public:
  virtual ~SubjectRepository() = default;
  /// Finds a subject by primary key.
  virtual std::optional<Subject> find_by_id(Id id) = 0;
  /// Lists subjects whose names contain the supplied text.
  virtual std::vector<Subject> find_by_name(const std::string& name) = 0;
  /// Creates or updates a subject.
  virtual Subject save(Subject subject) = 0;
  /// Permanently removes a subject by primary key.
  virtual bool remove(Id id) = 0;
};

class CourseRepository {
 public:
  virtual ~CourseRepository() = default;
  /// Finds a course by primary key.
  virtual std::optional<Course> find_by_id(Id id) = 0;
  /// Lists courses whose names contain the supplied text.
  virtual std::vector<Course> find_by_name(const std::string& name) = 0;
  /// Lists courses created at the supplied UTC timestamp.
  virtual std::vector<Course> find_by_created_at(const std::string& created_at) = 0;
  /// Creates or updates a course.
  virtual Course save(Course course) = 0;
  /// Verifies that a teacher owns a course.
  virtual bool is_teacher_for(Id user_id, Id course_id) = 0;
  /// Permanently removes a course by primary key.
  virtual bool remove(Id id) = 0;
};

class TeacherConfigurationRepository {
 public:
  virtual ~TeacherConfigurationRepository() = default;
  /// Finds the policy configuration scoped to one teacher and course.
  virtual std::optional<TeacherConfiguration> find_for_teacher_course(Id teacher_id, Id course_id) = 0;
  /// Creates or updates a teacher configuration.
  virtual TeacherConfiguration save(TeacherConfiguration configuration) = 0;
  /// Permanently removes a configuration by primary key.
  virtual bool remove(Id id) = 0;
};

class EnrollmentRepository {
 public:
  virtual ~EnrollmentRepository() = default;
  /// Checks whether a user is enrolled in a course.
  virtual bool exists(Id user_id, Id course_id) = 0;
  /// Enrolls a user in a course.
  virtual bool create(Id user_id, Id course_id) = 0;
  /// Removes a user's enrollment from a course.
  virtual bool remove(Id user_id, Id course_id) = 0;
};

class QuestionRepository {
 public:
  virtual ~QuestionRepository() = default;
  /// Searches questions within a course, optionally limited to approved items.
  virtual std::vector<Question> search(Id course_id, const std::string& query, bool approved_only) = 0;
  /// Finds a question by primary key.
  virtual std::optional<Question> find_by_id(Id id) = 0;
  /// Lists questions in a course with a workflow status.
  virtual std::vector<Question> find_by_status(Id course_id, QuestionStatus status) = 0;
  /// Lists questions in a course whose text contains the supplied content.
  virtual std::vector<Question> find_by_content(Id course_id, const std::string& content) = 0;
  /// Lists questions in a course created at the supplied UTC timestamp.
  virtual std::vector<Question> find_by_created_at(Id course_id, const std::string& created_at) = 0;
  /// Creates or updates a question.
  virtual Question save(Question question) = 0;
  /// Changes a question's status to archived.
  virtual bool archive(Id id) = 0;
  /// Permanently removes a question by primary key.
  virtual bool remove(Id id) = 0;
};

class LearningSessionRepository {
 public:
  virtual ~LearningSessionRepository() = default;
  /// Finds a learning session by primary key.
  virtual std::optional<LearningSession> find_by_id(Id id) = 0;
  /// Creates or updates a learning session.
  virtual LearningSession save(LearningSession session) = 0;
  /// Permanently removes a learning session by primary key.
  virtual bool remove(Id id) = 0;
};

class MessageRepository {
 public:
  virtual ~MessageRepository() = default;
  /// Returns the newest messages for a session up to the requested limit.
  virtual std::vector<Message> recent_for_session(Id session_id, int limit) = 0;
  /// Lists messages in a session whose content contains the supplied text.
  virtual std::vector<Message> find_by_content(Id session_id, const std::string& content) = 0;
  /// Lists messages in a session created at the supplied UTC timestamp.
  virtual std::vector<Message> find_by_created_at(Id session_id, const std::string& created_at) = 0;
  /// Creates or updates a message.
  virtual Message save(Message message) = 0;
  /// Permanently removes a message by primary key.
  virtual bool remove(Id id) = 0;
};

class RoadmapRepository {
 public:
  virtual ~RoadmapRepository() = default;
  /// Finds a roadmap by primary key.
  virtual std::optional<Roadmap> find_by_id(Id id) = 0;
  /// Finds the roadmap belonging to a user in a course.
  virtual std::optional<Roadmap> find_for_user_course(Id user_id, Id course_id) = 0;
  /// Creates or updates a learning roadmap.
  virtual Roadmap save(Roadmap roadmap) = 0;
  /// Permanently removes a roadmap by primary key.
  virtual bool remove(Id id) = 0;
};

class ProgressRepository {
 public:
  virtual ~ProgressRepository() = default;
  /// Lists a user's learning progress in a course.
  virtual std::vector<LearningProgress> list(Id user_id, Id course_id) = 0;
  /// Creates or updates a learning progress entry.
  virtual LearningProgress save(LearningProgress progress) = 0;
  /// Permanently removes a progress entry by primary key.
  virtual bool remove(Id id) = 0;
};

class AssessmentRepository {
 public:
  virtual ~AssessmentRepository() = default;
  /// Finds an assessment by primary key.
  virtual std::optional<ThinkingAssessment> find_by_id(Id id) = 0;
  /// Lists a student's assessments in one course.
  virtual std::vector<ThinkingAssessment> list_for_user_course(Id user_id, Id course_id) = 0;
  /// Lists all assessments for a course.
  virtual std::vector<ThinkingAssessment> list_for_course(Id course_id) = 0;
  /// Searches assessments for text relevant to a course.
  virtual std::vector<ThinkingAssessment> find_in_course(Id course_id, const std::string& query) = 0;
  /// Lists assessments with a review status in a course.
  virtual std::vector<ThinkingAssessment> find_by_status(Id course_id, const std::string& review_status) = 0;
  /// Lists assessments in a course created at the supplied UTC timestamp.
  virtual std::vector<ThinkingAssessment> find_by_created_at(Id course_id, const std::string& created_at) = 0;
  /// Creates an assessment or saves its teacher review.
  virtual ThinkingAssessment save(ThinkingAssessment assessment) = 0;
  /// Permanently removes an assessment by primary key.
  virtual bool remove(Id id) = 0;
};

class DocumentRepository {
 public:
  virtual ~DocumentRepository() = default;
  /// Lists all documents for a course.
  virtual std::vector<Document> find_by_course(Id course_id) = 0;
  /// Creates or updates a source document.
  virtual Document save(Document document) = 0;
  /// Permanently removes a document by primary key.
  virtual bool remove(Id id) = 0;
};

class DocumentChunkRepository {
 public:
  virtual ~DocumentChunkRepository() = default;
  /// Lists chunks of a document in source order.
  virtual std::vector<DocumentChunk> list_for_document(Id document_id) = 0;
  /// Lists document chunks that match the supplied query in a set of documents from one course.
  virtual std::vector<DocumentChunk> find_in_documents(Id course_id, const std::vector<Id>& document_ids,
                                                    const std::string& query) = 0;
  /// Lists document chunks whose content contains the supplied text.
  virtual std::vector<DocumentChunk> find_by_content(Id document_id, const std::string& content) = 0;
  /// Creates or updates a document chunk.
  virtual DocumentChunk save(DocumentChunk chunk) = 0;
  /// Permanently removes a document chunk by primary key.
  virtual bool remove(Id id) = 0;
};
}  // namespace edu_ai::repositories
