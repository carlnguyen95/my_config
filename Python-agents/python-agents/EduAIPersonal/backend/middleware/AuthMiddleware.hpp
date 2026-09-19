#pragma once

#include <optional>
#include "backend/models/Domain.hpp"

namespace edu_ai::middleware {
struct RequestIdentity { models::Id user_id{}; models::Role role{models::Role::Student}; };
class AuthMiddleware { public: virtual ~AuthMiddleware() = default; virtual std::optional<RequestIdentity> authenticate(const char* authorization_header) const = 0; };
}

