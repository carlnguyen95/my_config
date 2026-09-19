cmake_minimum_required(VERSION 3.21)
project(edu_ai VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EDU_AI_BUILD_TESTS "Build the skeleton tests" ON)
option(EDU_AI_ENABLE_DROGON "Enable the HTTP adapter once Drogon is installed" OFF)

add_library(edu_ai_core
    backend/ai/AIEngine.cpp
    backend/ai/PolicyEngine.cpp
    backend/ai/PromptBuilder.cpp
    backend/ai/ToolRegistry.cpp
    backend/rag/Chunker.cpp
)
target_include_directories(edu_ai_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(edu_ai_core PUBLIC cxx_std_20)

add_executable(edu_ai backend/main.cpp)
target_link_libraries(edu_ai PRIVATE edu_ai_core)

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
endif()

