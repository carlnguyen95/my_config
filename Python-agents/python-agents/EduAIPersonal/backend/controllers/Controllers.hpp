#pragma once

// HTTP adapters belong here. With Drogon enabled, each controller should parse
// a request, obtain middleware identity, call a service, and map Result to JSON.
namespace edu_ai::controllers {
class AuthController {};
class CourseController {};
class LearningController {};
class QuestionController {};
class RoadmapController {};
class ProgressController {};
class AssessmentController {};
}  // namespace edu_ai::controllers

