# Frontend change log

## 2026-09-21 — Temporary React/Vite frontend integration

- Registered the temporary React/Vite frontend in the root deployment workflow.
- Extended `scripts/setup.sh` to check Node.js, npm, and the frontend dependency tree. During normal setup it creates
  `frontend/.env` from the example when needed and runs `npm ci` when a lockfile exists, otherwise `npm install`.
- Extended `scripts/run.sh` with `frontend` and `backend` targets. `scripts/run.sh frontend` launches the Vite/Express
  development server at `http://localhost:3000`.
- Updated `frontend/server.ts` to load `frontend/.env` through `dotenv/config`.
- Updated `frontend/.env.example`, the root README, and `frontend/README.md`. `GEMINI_API_KEY` is optional; the mock UI
  can run without it.

## Verification

- `npm run lint` passed.
- `npm run build` passed.
- `scripts/run.sh frontend` was smoke-tested successfully; `GET /api/health` returned a successful response.

## 2026-09-21 — Frontend/backend API compatibility tests

- Added API contract tests for base URL selection, bearer-token handling, C++ `{ success, data }` envelopes, error
  responses, chat `answer` mapping, and course-scoped roadmap status updates.
- Added HTTP smoke tests for the temporary frontend mock API. Collection endpoints accept both the current direct-array
  mock payload and the object-wrapped payload used by backend integrations.
- Backend HTTP smoke tests are opt-in through `BE_BASE_URL`, because the Drogon HTTP server is not enabled in the default
  local core build.

## 2026-09-21 — C++/Drogon API compatibility boundary

- Added `frontend/src/api.ts`. `VITE_API_BASE_URL` is now the optional C++ API origin; when it is empty, the existing
  Express mock API remains the fallback. The client attaches the saved Bearer token and accepts both plain resource JSON
  and the C++ adapter's `{ "success": true, "data": ... }` envelope.
- Routed frontend API calls through the compatibility client. In particular, chat now maps the C++ `answer` response to
  an assistant message (the UI previously only rendered a non-existent `reply` field), and roadmap topic updates use the
  course ID instead of the roadmap ID.
- Documented backend selection and current Drogon coverage limits in `frontend/.env.example` and `frontend/README.md`.
- Updated the frontend `esbuild` version to `^0.28.0` and added `package-lock.json`, resolving the Vite 8 peer-dependency
  conflict that prevented a clean `npm install`.

## Verification

- `npm run lint` passed.
- `npm run build` passed.
- Smoke-tested the existing mock service on `http://127.0.0.1:3000`: `GET /api/health` and
  `POST /api/learning/chat` both returned successful JSON; chat returned the `answer` contract now handled by the UI.

## Remaining C++ adapter gaps

- The current Drogon adapter is not yet a full substitute for the temporary API: teacher dashboard/RAG inspection,
  question generation and batch import, personalization, course creation/deletion, and complete chat persistence still
  require backend routes before the UI can use them without mock fallback.

## 2026-09-21 — Session-aware header and logout

- Added `authSession.ts` to centralize saving, restoring and clearing the active user ID and optional bearer token.
- The header now renders a login CTA only when no session exists. For an active session it renders the current user's
  avatar, name and email in a focused account menu with logout only; it no longer exposes the complete user list or a
  switch-account/login control in that menu.
- Logout calls the API when available, clears local identity and token even if that call fails, clears user-scoped UI
  data, returns to the dashboard, and leaves the header in its unauthenticated CTA state.

## 2026-09-21 — Frontend/backend compatibility contract tests

- Added `frontend/tests/api_contract.test.ts` and the `npm run test:contract` command.
- Extracted the chat response normalizer into `frontend/src/chatResponse.ts` so both the temporary Express `reply` and
  Drogon `answer` formats are covered by executable tests.
- Added explicit helpers for API base URL composition and the course-scoped roadmap-topic status route.

## Verification

- `npm run lint` passed.
- `npm run build` passed (Vite emitted only its existing future `__dirname` configuration warning).
- `npm run test:contract` passed: 9/9 tests covering base URL construction, bearer-token forwarding and override,
  C++ `{ success, data }` envelope unwrapping, API errors, chat reply normalization, and course-scoped roadmap routes.

## 2026-09-21 — Login/logout HTTP integration coverage

- Extended `tests/FE/frontend_backend_api_test.mjs` and added `npm run test:api`.
- The mock-frontend suite now verifies login user identity used by the header (name, role, avatar), a returned token used
  as a Bearer header for profile retrieval, and the logout endpoint's successful response.
- `BE_BASE_URL` continues to opt into the real C++ backend checks; it was unset during this run, so those checks were
  skipped rather than routed to the mock server.

## Verification

- `npm run test:api` passed: 6 FE HTTP tests passed; 6 optional BE tests skipped because `BE_BASE_URL` is not configured.

## Known UI-session limitation

- This is HTTP-level coverage, not a browser-state assertion. `App.tsx` currently initializes a hard-coded user and
  `handleLogout` only removes `edu_ai_token` before reopening the login modal; it does not clear `edu_ai_user_id` or
  transition `currentUser` to an unauthenticated state. Therefore the UI cannot yet be truthfully tested as
  “guest → login updates header/avatar → logout restores guest CTA” without a separate authenticated/guest state
  implementation.

## 2026-09-21 — Frontend runtime diagnosis and log policy

- Identified that the active process on port 3000 was stale and not the current project frontend implementation.
- Verified root cause by checking live responses: `/api/health` returned JSON but `/api/users` and `/api/provider-state`
  returned Vite HTML instead of app JSON, confirming that the running server did not match the code in `frontend/server.ts`.
- Restarted the correct frontend process from the actual project directory and re-verified the API contract on port 3000.
- Confirmed that the active frontend now serves JSON payloads for the user list and provider state, which restored the UI's expected data flow.
- Updated the project log policy so operational notes and task updates are recorded directly in the FE/BE change logs instead of only in script execution logs.

## Verification

- Confirmed that `curl http://127.0.0.1:3000/api/users` returned a JSON array.
- Confirmed that `curl http://127.0.0.1:3000/api/provider-state` returned a JSON object.
- The frontend API contract for the active local app is now restored and the process is aligned with the current source tree.

## 2026-09-21 — Session UI test completion

- Supersedes the earlier UI-session limitation: `authSession.ts`, `App.tsx`, and `Header.tsx` now model an explicit
  authenticated state and clear both `edu_ai_user_id` and `edu_ai_token` during logout.
- Extended `frontend/tests/api_contract.test.ts` with server-rendered header checks for the unauthenticated login CTA
  and the authenticated account identity/avatar. The account menu API now receives only the active user, so it cannot
  render the previous multi-user account list.

## Verification

- `npm run lint` passed.
- `npm run build` passed (only the existing Vite `__dirname` future-compatibility warning was emitted).
- `npm run test:contract` passed: 13/13 tests, including session persistence/clearing and header CTA/account states.
- The optional `BE_BASE_URL=https://127.0.0.1:8443` smoke target was unavailable at final verification; the temporary
  Drogon process was no longer listening and its temporary database had no seed login account. No false positive was
  recorded for the real-backend login flow.

## 2026-09-21 — Credential-only login modal

- Removed the “Tài Khoản Mẫu” and “Đăng Ký Mới” tabs, including their quick-account selection and registration forms.
- The modal now exposes one email/password form only; sample credentials live in the test database rather than the UI.
