#pragma once

#include <string>

#include "backend/repositories/Repositories.hpp"
#include "backend/services/ServiceContracts.hpp"

namespace edu_ai::services {

class PasswordHasher {
 public:
  /// Derives a salted PBKDF2-SHA256 password hash.
  std::string hash(const std::string& password) const;
  /// Verifies a password against the application's encoded hash format.
  bool verify(const std::string& password, const std::string& encoded_hash) const;
};

class JwtTokenIssuer {
 public:
  explicit JwtTokenIssuer(std::string secret) : secret_(std::move(secret)) {}

  /// Issues a signed, time-limited JWT for an authenticated user.
  std::string issue(const models::User& user) const;

 private:
  std::string secret_;
};

class DefaultAuthService final : public AuthService {
 public:
  DefaultAuthService(repositories::UserRepository& users, PasswordHasher hasher, JwtTokenIssuer issuer)
      : users_(users), hasher_(std::move(hasher)), issuer_(std::move(issuer)) {}

  /// Implements account registration with validation and duplicate checks.
  common::Result<User> register_user(const std::string& name, const std::string& email, const std::string& password,
                                     Role role) override;
  /// Implements credential validation and token issuance.
  common::Result<std::string> login(const std::string& email, const std::string& password) override;

 private:
  repositories::UserRepository& users_;
  PasswordHasher hasher_;
  JwtTokenIssuer issuer_;
};

}  // namespace edu_ai::services
