#pragma once

#include "backend/models/Domain.hpp"

namespace edu_ai::middleware {
class RoleMiddleware {
 public:
  /// Reports whether an actual role satisfies the required endpoint role.
  static bool allows(models::Role actual, models::Role required) {
    return actual == models::Role::Admin || actual == required;
  }
};
}  // namespace edu_ai::middleware
