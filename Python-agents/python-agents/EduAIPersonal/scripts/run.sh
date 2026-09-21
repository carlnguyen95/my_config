#!/usr/bin/env sh
set -eu

usage() {
  echo "Usage: scripts/run.sh [backend|frontend|stop-frontend]" >&2
}

target=${1:-backend}
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)

case "$target" in
  backend)
    cd "$project_dir"
    if [ -f .env ]; then
      set -a
      . ./.env
      set +a
    fi
    cmake --build build
    exec ./build/edu_ai
    ;;
  frontend)
    if ! command -v npm >/dev/null 2>&1; then
      echo "npm is required to run the frontend. Run scripts/setup.sh after installing Node.js." >&2
      exit 1
    fi
    if [ ! -d "$project_dir/frontend/node_modules" ]; then
      echo "Frontend dependencies are absent. Run scripts/setup.sh or: (cd frontend && npm install)" >&2
      exit 1
    fi
    cd "$project_dir/frontend"
    exec npm run dev
    ;;
  stop-frontend|stop-fe|stop)
    pids=""

    if command -v lsof >/dev/null 2>&1; then
      pids=$(lsof -tiTCP:3000 -sTCP:LISTEN 2>/dev/null || true)
    elif command -v fuser >/dev/null 2>&1; then
      pids=$(fuser -n tcp 3000 2>/dev/null || true)
    else
      pids=$(ps -eo pid,comm --no-headers | awk '$2 ~ /(node|tsx|vite)/ {print $1}')
    fi

    if [ -n "$pids" ]; then
      echo "$pids" | xargs -r kill -9
      echo "Stopped FE process on port 3000."
    else
      echo "No FE process found on port 3000."
    fi
    ;;
  --help|-h)
    usage
    exit 0
    ;;
  *)
    usage
    exit 2
    ;;
esac
