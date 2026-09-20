#pragma once

#include <optional>
#include <string>

namespace edu_ai::common {

enum class ErrorCode {
  InvalidRequest,
  Unauthenticated,
  PermissionDenied,
  NotFound,
  Conflict,
  AiProviderError,
  ToolNotFound,
  ToolArgumentError,
  ToolPermissionDenied,
  AssessmentError,
  DatabaseError
};

struct Error {
  ErrorCode code;
  std::string message;
};

template <typename T>
struct Result {
  std::optional<T> value;
  std::optional<Error> error;

  /// Reports whether the operation produced a value instead of an error.
  [[nodiscard]] bool ok() const {
    return value.has_value();
  }

  /// Constructs a successful result containing the supplied value.
  static Result success(T item) {
    return {.value = std::move(item)};
  }

  /// Constructs a failed result with a machine-readable code and message.
  static Result failure(ErrorCode code, std::string message) {
    return {.error = Error{code, std::move(message)}};
  }
};

}  // namespace edu_ai::common
