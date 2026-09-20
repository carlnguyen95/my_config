#pragma once

#include <iostream>
#include <string>

namespace edu_ai::tests {

class Suite {
 public:
  /// Creates a named test suite and begins its structured output.
  explicit Suite(std::string name) : name_(std::move(name)) {
    std::cout << "\n[SUITE] " << name_ << '\n';
  }

  /// Prints the human-readable description of the next assertion group.
  void scenario(const std::string& description) {
    description_ = description;
    std::cout << "  [CHECK] " << description_ << '\n';
  }

  /// Records and reports one assertion with its source expression.
  void check(bool passed, const char* expression, const char* file, int line) {
    if (passed) {
      ++passed_;
      std::cout << "  [PASS] " << description_ << " | " << expression << '\n';
      return;
    }
    ++failed_;
    std::cerr << "  [FAIL] " << description_ << " | " << expression << " (" << file << ':' << line << ")\n";
  }

  /// Prints the suite summary and returns a process exit code.
  int finish() const {
    std::cout << "[SUMMARY] " << name_ << ": " << passed_ << " passed, " << failed_ << " failed\n";
    return failed_ == 0 ? 0 : 1;
  }

 private:
  std::string name_;
  std::string description_{"Unlabelled check"};
  int passed_{0};
  int failed_{0};
};

}  // namespace edu_ai::tests

#define CHECK(SUITE, EXPRESSION) (SUITE).check(static_cast<bool>(EXPRESSION), #EXPRESSION, __FILE__, __LINE__)
