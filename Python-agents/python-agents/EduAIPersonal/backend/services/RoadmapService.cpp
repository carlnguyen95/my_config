#include "backend/services/RoadmapService.hpp"
#include "backend/services/ServiceSupport.hpp"

namespace edu_ai::services {
using namespace detail;

/**
 * @brief Returns a visible roadmap.
 * @param actor_id Requester ID.
 * @param user_id Roadmap owner.
 * @param course_id Course ID.
 * @return Roadmap or error.
 */
common::Result<Roadmap> DefaultRoadmapService::get(Id actor_id, Id user_id, Id course_id) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<Roadmap>();
  if (current->id != user_id && !can_manage_course(*current, courses_, course_id))
    return denied<Roadmap>();
  if (current->id == user_id && !can_access_course(*current, courses_, enrollments_, course_id))
    return denied<Roadmap>();
  const auto roadmap = roadmaps_.find_for_user_course(user_id, course_id);
  return roadmap ? common::Result<Roadmap>::success(*roadmap)
                 : common::Result<Roadmap>::failure(common::ErrorCode::NotFound, "Roadmap was not found.");
}

/**
 * @brief Validates and saves a roadmap.
 * @param actor_id Teacher ID.
 * @param roadmap Roadmap fields.
 * @return Saved roadmap or error.
 */
common::Result<Roadmap> DefaultRoadmapService::save(Id actor_id, Roadmap roadmap) {
  const auto current = actor(users_, actor_id);
  if (!current)
    return missing_actor<Roadmap>();
  if (!can_manage_course(*current, courses_, roadmap.course_id))
    return denied<Roadmap>();
  if (roadmap.title.empty() || roadmap.content_json.empty())
    return common::Result<Roadmap>::failure(common::ErrorCode::InvalidRequest,
                                            "Roadmap title and content are required.");
  return common::Result<Roadmap>::success(roadmaps_.save(std::move(roadmap)));
}

/**
 * @brief Updates an existing roadmap through the validated save flow.
 * @param actor_id Authenticated teacher ID.
 * @param roadmap Existing roadmap with its ID.
 * @return Updated roadmap or an error.
 */
common::Result<Roadmap> DefaultRoadmapService::update(Id actor_id, Roadmap roadmap) {
  if (roadmap.id == 0)
    return common::Result<Roadmap>::failure(common::ErrorCode::InvalidRequest, "Roadmap ID is required for update.");
  const auto existing = roadmaps_.find_by_id(roadmap.id);
  if (!existing)
    return common::Result<Roadmap>::failure(common::ErrorCode::NotFound, "Roadmap was not found.");
  if (roadmap.user_id != existing->user_id || roadmap.course_id != existing->course_id)
    return common::Result<Roadmap>::failure(common::ErrorCode::InvalidRequest,
                                            "Roadmap owner and course cannot be changed.");
  return save(actor_id, std::move(roadmap));
}
}  // namespace edu_ai::services
