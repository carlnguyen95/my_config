# Edu AI — Mentor's Assistant

Skeleton for the 3-day MVP specified in `edu_ai_3day_mvp_codex_spec.md`.

The codebase uses C++20 and CMake. Domain, service, repository, AI, RAG, and HTTP-adapter boundaries are deliberately separate, so the MVP can begin as a CLI/process and attach Drogon without moving business logic into controllers.

## Current status

The repository and service layers are implemented for the core MVP data model. Local Ollama generation, a
thread-safe round-robin model router, and an opt-in latency benchmark are available. RAG retrieval, persistent chat
orchestration, and the production HTTP adapter are still being completed.

## Prerequisites

The deployment check in `scripts/setup.sh` validates the following items:

| Requirement | Purpose |
| --- | --- |
| POSIX shell, CMake 3.21+, and a C++20 compiler | Configure and build the C++ application. |
| SQLite3, OpenSSL Crypto, and JsonCpp development packages | Required CMake dependencies. |
| `jq` | Reads and validates the local AI model pool in `config/ai.json`. |
| Ollama CLI and a running Ollama service | Runs local models and the round-robin benchmark. |
| Drogon with `drogon_ctl` and CMake package support | Required when deploying the HTTP API. |
| Node.js and npm | Required by the temporary React/Vite frontend in `frontend/`. |

For a Fedora/RHEL-like host, install the basic build dependencies with your package manager, for example:

```sh
sudo dnf install cmake gcc-c++ sqlite-devel openssl-devel jsoncpp-devel jq
```

Install Ollama and Drogon from their supported distribution packages or build instructions. After installation,
confirm that `ollama list` and `drogon_ctl --version` both work. The deployment script reports a missing Drogon
installation, but finding Drogon alone does not mean the unfinished HTTP controller adapter is production-ready.

The C++ backend listens on HTTPS port `8443` when Drogon is enabled. It requires `config/ssl/server.crt` and
`config/ssl/server.key`; `scripts/setup.sh --check-only` reports them when missing. For local development only,
create a self-signed certificate with:

```sh
openssl req -x509 -nodes -newkey rsa:2048 \
  -keyout config/ssl/server.key -out config/ssl/server.crt \
  -days 365 -subj '/CN=localhost'
```

Set a random `EDU_AI_JWT_SECRET` of at least 32 characters in `.env` before starting HTTPS; `scripts/run.sh backend`
loads this file for the server process.

Install a current Node.js LTS release with npm before setting up the frontend. Setup uses `npm ci` when a frontend
lockfile is present, and falls back to `npm install` otherwise.

### Required local model pool

The default round-robin pool in `config/ai.json` is:

```text
gemma4
gemma4:12b
qwen3:8b
```

Ollama treats `gemma4` as `gemma4:latest` when that tag is installed. Install the pool with:

```sh
ollama pull gemma4:latest
ollama pull gemma4:12b
ollama pull qwen3:8b
```

Ensure the Ollama service is running before deployment. On systems where it is not managed by a service manager, run:

```sh
ollama serve
```

## Deployment setup

Check the deployment host before building:

```sh
scripts/setup.sh --check-only
```

The command lists installed tools and Ollama models, then prints an `ollama pull <tag>` command for each missing model.
It exits with `0` only when every checked prerequisite is ready, `1` when something is missing, and `2` for invalid
arguments. It does not modify the machine or download models.

After the check passes, perform the normal setup:

```sh
scripts/setup.sh
```

The normal setup creates `.env` and `frontend/.env` from their examples when needed, installs frontend packages with
`npm install` (or `npm ci` when a lockfile is present), configures and builds the project, initializes the SQLite
database, and then exits non-zero if the deployment inventory is still incomplete.

## Running the temporary frontend

The temporary frontend is a React/Vite SPA served by its Express development server at
<http://localhost:3000>. It uses in-memory demo data and is not connected to the C++ backend yet.

After setup, start it with:

```sh
scripts/run.sh frontend
```

`GEMINI_API_KEY` in `frontend/.env` is optional. Leave it empty to use the mock UI; add a valid key only when testing
the Gemini-backed demo generation endpoints. To run the C++ core as before, use:

```sh
scripts/run.sh backend
```

The backend then serves `https://localhost:8443/api/health`. A browser connecting from the Vite dev server can point
the frontend at it with `VITE_API_BASE_URL=https://localhost:8443`; accept the self-signed certificate locally first.

## Layout

- `backend/models`: domain types and roles/policies.
- `backend/services`: application use-case contracts.
- `backend/repositories`: one SQLite repository per database table; SQL belongs here.
- `backend/ai`: provider abstraction, prompt/policy/tool orchestration.
- `backend/rag`: document chunking, embedding, and retrieval boundaries.
- `backend/controllers` and `backend/middleware`: thin HTTP adapter boundary.
- `database/init.sql`: SQLite schema.
- `config`: runtime defaults and centralized prompts.

## Manual build and test

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

For round-robin latency measurements, see [docs/round_robin_local_models.md](docs/round_robin_local_models.md).

## Next vertical slice

1. Add the Drogon routes and auth middleware for register/login/me.
2. Complete the policy → prompt → tool → storage path on top of `OllamaProvider`.
3. Add document ingestion and RAG retrieval services.
