#pragma once

#include "backend/models/Domain.hpp"

namespace edu_ai::middleware {
class RoleMiddleware { public: static bool allows(models::Role actual, models::Role required) { return actual == models::Role::Admin || actual == required; } };
}

