# Edu AI — Mentor's Assistant

Skeleton for the 3-day MVP specified in `edu_ai_3day_mvp_codex_spec.md`.

The codebase uses C++20 and CMake. Domain, service, repository, AI, RAG, and HTTP-adapter boundaries are deliberately separate, so the MVP can begin as a CLI/process and attach Drogon without moving business logic into controllers.

## Current status

This commit establishes the project shape and compileable core contracts only. Authentication, persistence, HTTP endpoints, Ollama requests, and RAG retrieval are placeholders for the next vertical-slice implementation.

## Layout

- `backend/models`: domain types and roles/policies.
- `backend/services`: application use-case contracts.
- `backend/repositories`: persistence contracts; SQL belongs here.
- `backend/ai`: provider abstraction, prompt/policy/tool orchestration.
- `backend/rag`: document chunking, embedding, and retrieval boundaries.
- `backend/controllers` and `backend/middleware`: thin HTTP adapter boundary.
- `database/init.sql`: SQLite schema.
- `config`: runtime defaults and centralized prompts.

## Build

```sh
cp .env.example .env
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/edu_ai
```

To attach Drogon later, install it and configure with `-DEDU_AI_ENABLE_DROGON=ON`. Controllers should remain adapters and call services only.

## Next vertical slice

1. Implement SQLite connection/migrations and repositories.
2. Implement password hashing, signed session/JWT handling, and auth middleware.
3. Add the Drogon routes for register/login/me and learning chat.
4. Implement `OllamaProvider`, then complete the policy → prompt → tool → storage path.

