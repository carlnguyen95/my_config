<div align="center">
<img width="1200" height="475" alt="GHBanner" src="https://ai.google.dev/static/site-assets/images/share-ais-513315318.png" />
</div>

# Edu AI frontend (temporary UI)

The frontend is a React/Vite application served together with a temporary
Express API in `server.ts`. It can run entirely in mock/fallback mode while the
C++ Drogon HTTP adapter is being completed.

## Run locally

1. Copy the environment template: `cp .env.example .env`
2. Install dependencies: `npm install`
3. Start it: `npm run dev`

The default URL is `http://localhost:3000`. `GEMINI_API_KEY` is optional; if it
is absent, the temporary API uses its deterministic local fallback responses.

## Connect to the C++ backend

Set `VITE_API_BASE_URL` before building the frontend, for example:

```dotenv
VITE_API_BASE_URL="https://localhost:8443"
```

The client then sends `/api/*` calls to that origin and attaches the stored
Bearer token when one is available. It understands both the temporary API's
resource JSON and the C++ adapter's `{ "success": true, "data": ... }`
envelope. The Drogon adapter currently does not implement every temporary-API
endpoint (notably teacher dashboard, question generation and personalization),
so leave `VITE_API_BASE_URL` blank for the complete demo experience.
