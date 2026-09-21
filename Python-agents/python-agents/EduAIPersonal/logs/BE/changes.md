# Backend change log

## 2026-09-21 — Local AI round-robin deployment

- Added a thread-safe round-robin provider for the Ollama model pool:
  `gemma4`, `gemma4:12b`, and `qwen3:8b`.
- Added the local Ollama provider implementation, a concurrent 5/10-request latency benchmark, and unit coverage for
  round-robin ordering and concurrent access.
- Updated `config/ai.json` and `docs/round_robin_local_models.md` with the local model routing configuration and
  benchmark instructions.
- Extended `scripts/setup.sh` with `--check-only`. It checks CMake, C++ compiler, `jq`, Ollama connectivity and
  configured models, plus Drogon availability; it prints the exact `ollama pull` command for missing models.
- Updated the root README with backend prerequisites, deployment checks, expected exit codes, and Ollama model setup.

## Verification

- Core C++ build and test suite passed with Drogon HTTP support disabled.
- The deployment inventory found Drogon `1.9.13`; models `gemma4:12b` and `qwen3:8b` must be installed on hosts where
  they are absent.

## 2026-09-21 — Backend runtime verification and log discipline

- Verified that the C++ backend can still be started successfully in HTTPS mode with the generated self-signed certificate.
- Confirmed the backend health and service startup path while the frontend runtime issue was being diagnosed separately.
- Kept logs focused on implementation and runtime context in the project log files, rather than recording only shell execution traces.
- Documented the distinction between the FE runtime issue (stale process on port 3000) and the BE start-up path (HTTPS server on 8443).
- Established the rule that each project update and diagnosis step should be appended to the FE/BE change log files for traceability.

## Verification

- Backend HTTPS startup path was validated with the configured cert and key.
- Runtime debugging notes remain aligned with the codebase and the live deployment behavior rather than with incidental shell invocation logs.

## 2026-09-21 — Drogon HTTPS runtime

- Replaced Drogon's auto-created controller macros with a dependency-aware manual router. The HTTPS process now owns
  its SQLite repositories and service dependencies instead of relying on controllers that Drogon cannot construct.
- Registered HTTPS health, login, current-user, course, and document routes. Added development CORS headers for the
  Vite server at `http://localhost:3000`.
- Added `--database <path>` to the HTTPS executable. The runtime initializes its SQLite schema before accepting
  requests.
- Setup now enables Drogon explicitly and checks for the certificate/key pair required by HTTPS startup.
- HTTPS startup now requires `EDU_AI_JWT_SECRET` (at least 32 characters) and uses it to sign login tokens;
  `scripts/run.sh backend` loads it from `.env`.

## Verification

- A clean build with `-DEDU_AI_ENABLE_DROGON=ON` passed.
- The server started with a temporary SQLite database and self-signed certificate; `curl -k
  https://127.0.0.1:8443/api/health` returned `{"status":"ok","transport":"https"}`.
- All seven C++ tests, including `drogon_api_test` with accepted and rejected login credentials, passed with Drogon
  enabled.

## 2026-09-21 — Manual-login test accounts

- Replaced six test-db identities with the frontend demonstration accounts and valid PBKDF2-SHA256 hashes for the
  shared local-only password `EduAI@2026`.

## 2026-09-21 — Drogon auto-route architecture

- Added `bootstrap::HttpRuntime` as the application composition root for SQLite repositories and HTTP services.
- Converted Auth and Course controllers back to Drogon's auto-created `HttpController` lifecycle. They receive their
  dependencies from `HttpRuntime`, and expose callback-based handlers through `ADD_METHOD_TO` at explicit `/api/...`
  paths.
- Reduced Router to infrastructure configuration only: runtime initialization, CORS, and HTTPS health. It no longer
  manually registers business endpoints.

## Verification

- HTTPS smoke test verified the automatically registered `/api/auth/login` and `/api/courses` routes against the reset
  database, in addition to `/api/health`.
