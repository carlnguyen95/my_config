#pragma once

#include <optional>
#include <string>

namespace edu_ai::common {

enum class ErrorCode {
  InvalidRequest, Unauthenticated, PermissionDenied, NotFound, Conflict,
  AiProviderError, ToolNotFound, ToolArgumentError, ToolPermissionDenied,
  AssessmentError, DatabaseError
};

struct Error { ErrorCode code; std::string message; };

template <typename T>
struct Result {
  std::optional<T> value;
  std::optional<Error> error;
  [[nodiscard]] bool ok() const { return value.has_value(); }
  static Result success(T item) { return {.value = std::move(item)}; }
  static Result failure(ErrorCode code, std::string message) {
    return {.error = Error{code, std::move(message)}};
  }
};

}  // namespace edu_ai::common

