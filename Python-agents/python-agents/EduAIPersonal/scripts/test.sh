#!/usr/bin/env sh
set -eu

test_group="${1:-all}"

cmake -S . -B build
cmake --build build

case "$test_group" in
  all)
    ctest --test-dir build -V
    ;;
  policy)
    ctest --test-dir build -V -R '^policy_engine_test$'
    ;;
  tool)
    ctest --test-dir build -V -R '^tool_registry_test$'
    ;;
  repository)
    ctest --test-dir build -V -R '^repository_test$'
    ;;
  service)
    ctest --test-dir build -V -R '^service_test$'
    ;;
  seed)
    ctest --test-dir build -V -R '^seed_database_test$'
    ;;
  policy_engine_test|tool_registry_test|repository_test|service_test|seed_database_test)
    ctest --test-dir build -V -R "^${test_group}$"
    ;;
  *)
    echo "Usage: $0 {all|policy|tool|repository|service|seed|<ctest-name>}" >&2
    exit 2
    ;;
esac
