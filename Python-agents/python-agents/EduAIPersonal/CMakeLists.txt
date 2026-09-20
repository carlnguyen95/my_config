cmake_minimum_required(VERSION 3.21)
project(edu_ai VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(EDU_AI_BUILD_TESTS "Build the skeleton tests" ON)
option(EDU_AI_ENABLE_DROGON "Enable the HTTP adapter once Drogon is installed" OFF)

find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)
find_package(jsoncpp CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_library(edu_ai_core
    backend/common/Utils.cpp
    backend/ai/AIEngine.cpp
    backend/ai/OllamaProvider.cpp
    backend/ai/PolicyEngine.cpp
    backend/ai/PromptBuilder.cpp
    backend/ai/RoundRobinProvider.cpp
    backend/ai/ToolRegistry.cpp
    backend/rag/Chunker.cpp
    backend/repositories/Database.cpp
    backend/repositories/UserRepository.cpp
    backend/repositories/SubjectRepository.cpp
    backend/repositories/CourseRepository.cpp
    backend/repositories/TeacherConfigurationRepository.cpp
    backend/repositories/EnrollmentRepository.cpp
    backend/repositories/QuestionRepository.cpp
    backend/repositories/LearningSessionRepository.cpp
    backend/repositories/MessageRepository.cpp
    backend/repositories/RoadmapRepository.cpp
    backend/repositories/ProgressRepository.cpp
    backend/repositories/AssessmentRepository.cpp
    backend/repositories/DocumentRepository.cpp
    backend/repositories/DocumentChunkRepository.cpp
    backend/services/AuthService.cpp
    backend/services/CourseService.cpp
    backend/services/TeacherConfigurationService.cpp
    backend/services/QuestionService.cpp
    backend/services/RoadmapService.cpp
    backend/services/ProgressService.cpp
    backend/services/AssessmentService.cpp
)
target_include_directories(edu_ai_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(edu_ai_core PUBLIC cxx_std_20)
target_link_libraries(edu_ai_core PUBLIC SQLite3::SQLite3 OpenSSL::Crypto Threads::Threads jsoncpp_lib)

add_executable(edu_ai backend/main.cpp)
target_link_libraries(edu_ai PRIVATE edu_ai_core)
target_compile_definitions(edu_ai PRIVATE EDU_AI_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(ollama_round_robin_benchmark backend/ai/OllamaRoundRobinBenchmark.cpp)
target_link_libraries(ollama_round_robin_benchmark PRIVATE edu_ai_core)

if(EDU_AI_ENABLE_DROGON)
    find_package(Drogon CONFIG REQUIRED)
    target_link_libraries(edu_ai PRIVATE Drogon::Drogon)
    target_compile_definitions(edu_ai PRIVATE EDU_AI_ENABLE_DROGON=1)
endif()

if(EDU_AI_BUILD_TESTS)
    enable_testing()
    add_executable(policy_engine_test tests/unit/PolicyEngineTest.cpp)
    target_link_libraries(policy_engine_test PRIVATE edu_ai_core)
    add_test(NAME policy_engine_test COMMAND policy_engine_test)

    add_executable(round_robin_provider_test tests/unit/RoundRobinProviderTest.cpp)
    target_link_libraries(round_robin_provider_test PRIVATE edu_ai_core)
    add_test(NAME round_robin_provider_test COMMAND round_robin_provider_test)

    add_executable(tool_registry_test tests/unit/ToolRegistryTest.cpp)
    target_link_libraries(tool_registry_test PRIVATE edu_ai_core)
    target_compile_definitions(tool_registry_test PRIVATE EDU_AI_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME tool_registry_test COMMAND tool_registry_test)

    add_executable(repository_test tests/unit/RepositoryTest.cpp)
    target_link_libraries(repository_test PRIVATE edu_ai_core)
    target_compile_definitions(repository_test PRIVATE EDU_AI_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME repository_test COMMAND repository_test)

    add_executable(service_test tests/unit/ServiceTest.cpp)
    target_link_libraries(service_test PRIVATE edu_ai_core)
    target_compile_definitions(service_test PRIVATE EDU_AI_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME service_test COMMAND service_test)

    add_executable(seed_database_test tests/integration/SeedDatabaseTest.cpp)
    target_link_libraries(seed_database_test PRIVATE edu_ai_core)
    target_compile_definitions(seed_database_test PRIVATE EDU_AI_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME seed_database_test COMMAND seed_database_test)
endif()
