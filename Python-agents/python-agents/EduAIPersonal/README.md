# Edu AI — Mentor's Assistant

Skeleton for the 3-day MVP specified in `edu_ai_3day_mvp_codex_spec.md`.

The codebase uses C++20 and CMake. Domain, service, repository, AI, RAG, and HTTP-adapter boundaries are deliberately separate, so the MVP can begin as a CLI/process and attach Drogon without moving business logic into controllers.

## Current status

The repository and service layers are implemented for the core MVP data model. HTTP endpoints, Ollama requests, RAG retrieval, and chat orchestration remain for the next vertical slice.

## Layout

- `backend/models`: domain types and roles/policies.
- `backend/services`: application use-case contracts.
- `backend/repositories`: one SQLite repository per database table; SQL belongs here.
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

Run tests with logs and summaries:

```sh
scripts/test.sh all
scripts/test.sh repository
scripts/test.sh seed
```

Initialize a database explicitly with:

```sh
./build/edu_ai --init-db data/edu_ai.db
```

To attach Drogon later, install it and configure with `-DEDU_AI_ENABLE_DROGON=ON`. Controllers should remain adapters and call services only.

## Next vertical slice

1. Add the Drogon routes and auth middleware for register/login/me.
2. Implement `OllamaProvider`, then complete the policy → prompt → tool → storage path.
3. Add document ingestion and RAG retrieval services.
