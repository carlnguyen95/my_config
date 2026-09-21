#!/usr/bin/env sh
set -eu

usage() {
  echo "Usage: scripts/setup.sh [--check-only]" >&2
}

check_only=false
case "${1:-}" in
  "") ;;
  --check-only) check_only=true ;;
  --help|-h)
    usage
    exit 0
    ;;
  *)
    usage
    exit 2
    ;;
esac

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)
cd "$project_dir"

if [ ! -f .env ]; then
  if [ "$check_only" = true ]; then
    echo "[WARN] .env is absent; checking config/ai.json defaults only."
  else
    cp .env.example .env
    echo "[OK] Created .env from .env.example"
  fi
fi

if [ -f .env ]; then
  set -a
  . ./.env
  set +a
fi

ai_config=${EDU_AI_AI_CONFIG:-config/ai.json}
frontend_dir="$project_dir/frontend"
build_prerequisites_ready=true
deployment_ready=true
frontend_present=false
frontend_prerequisites_ready=true

require_command() {
  command_name=$1
  purpose=$2
  if command -v "$command_name" >/dev/null 2>&1; then
    echo "[OK] $command_name ($purpose)"
  else
    echo "[MISSING] $command_name ($purpose)"
    deployment_ready=false
    if [ "$command_name" = cmake ] || [ "$command_name" = c++ ]; then
      build_prerequisites_ready=false
    fi
  fi
}

check_drogon() {
  if ! command -v drogon_ctl >/dev/null 2>&1; then
    echo "[MISSING] drogon_ctl (Drogon HTTP framework)"
    echo "  install Drogon with CMake package support before deploying the HTTP API."
    deployment_ready=false
    return
  fi

  drogon_version=$(drogon_ctl --version 2>/dev/null | awk '/^Version:/ { print $2; exit }')
  if [ -z "$drogon_version" ]; then
    echo "[MISSING] Working Drogon installation"
    deployment_ready=false
    return
  fi
  echo "[OK] Drogon HTTP framework (version $drogon_version)"
}

check_https_credentials() {
  tls_cert="$project_dir/config/ssl/server.crt"
  tls_key="$project_dir/config/ssl/server.key"
  if [ -f "$tls_cert" ] && [ -f "$tls_key" ]; then
    echo "[OK] HTTPS certificate and key"
    return
  fi

  echo "[MISSING] HTTPS certificate or key"
  echo "  expected: $tls_cert"
  echo "            $tls_key"
  echo "  local development: openssl req -x509 -nodes -newkey rsa:2048 -keyout config/ssl/server.key -out config/ssl/server.crt -days 365 -subj '/CN=localhost'"
  deployment_ready=false
}

check_frontend() {
  if [ ! -f "$frontend_dir/package.json" ]; then
    echo "[SKIP] Frontend package manifest not found"
    return
  fi

  frontend_present=true
  echo "Frontend inventory"

  if command -v node >/dev/null 2>&1; then
    echo "[OK] node (version $(node --version))"
  else
    echo "[MISSING] node (frontend runtime)"
    frontend_prerequisites_ready=false
    deployment_ready=false
  fi

  if command -v npm >/dev/null 2>&1; then
    echo "[OK] npm (version $(npm --version))"
  else
    echo "[MISSING] npm (frontend dependency manager)"
    frontend_prerequisites_ready=false
    deployment_ready=false
  fi

  if [ "$frontend_prerequisites_ready" = true ]; then
    if [ -d "$frontend_dir/node_modules" ] && (cd "$frontend_dir" && npm ls --depth=0 >/dev/null 2>&1); then
      echo "[OK] Frontend npm dependencies"
    else
      echo "[MISSING] Frontend npm dependencies"
      echo "  install: (cd frontend && npm install)"
      deployment_ready=false
    fi
  fi

  if [ ! -f "$frontend_dir/.env" ]; then
    echo "[WARN] frontend/.env is absent; the mock UI runs without Gemini, but AI generation needs GEMINI_API_KEY."
  fi
}

echo "Deployment inventory"
require_command cmake "build configuration"
require_command c++ "C++20 compiler"
require_command jq "AI configuration parser"
require_command ollama "local model runtime"
check_drogon
check_https_credentials
check_frontend

configured_models=""
if command -v jq >/dev/null 2>&1; then
  if [ ! -f "$ai_config" ]; then
    echo "[MISSING] AI config: $ai_config"
    deployment_ready=false
  elif ! jq -e '
      .ollama.routing.models
      | type == "array"
        and length > 0
        and all(.[]; type == "string" and length > 0)
    ' "$ai_config" >/dev/null; then
    echo "[MISSING] Valid ollama.routing.models in $ai_config"
    deployment_ready=false
  else
    configured_models=$(jq -r '.ollama.routing.models[]' "$ai_config")
    echo "[OK] AI model pool from $ai_config"
    while IFS= read -r model; do
      [ -n "$model" ] && echo "  - required: $model"
    done <<EOF
$configured_models
EOF
  fi
fi

installed_models=""
ollama_available=false
if command -v ollama >/dev/null 2>&1; then
  if ollama_output=$(ollama list 2>&1); then
    installed_models=$(printf '%s\n' "$ollama_output" | awk 'NR > 1 && NF { print $1 }')
    ollama_available=true
    echo "[OK] Ollama service is reachable"
    if [ -n "$installed_models" ]; then
      echo "  Installed models:"
      while IFS= read -r model; do
        [ -n "$model" ] && echo "  - $model"
      done <<EOF
$installed_models
EOF
    else
      echo "  Installed models: none"
    fi
  else
    echo "[MISSING] Ollama service is not reachable; start it with: ollama serve"
    deployment_ready=false
  fi
fi

if [ "$ollama_available" = true ] && [ -n "$configured_models" ]; then
  missing_models=""
  while IFS= read -r required_model; do
    [ -n "$required_model" ] || continue
    model_found=false
    while IFS= read -r installed_model; do
      [ "$installed_model" = "$required_model" ] && model_found=true
      if [ "${required_model#*:}" = "$required_model" ] && [ "$installed_model" = "$required_model:latest" ]; then
        model_found=true
      fi
    done <<EOF
$installed_models
EOF
    if [ "$model_found" = false ]; then
      if [ -n "$missing_models" ]; then
        missing_models="$missing_models
$required_model"
      else
        missing_models="$required_model"
      fi
    fi
  done <<EOF
$configured_models
EOF

  if [ -n "$missing_models" ]; then
    deployment_ready=false
    echo "[MISSING] Required Ollama models:"
    while IFS= read -r model; do
      [ -n "$model" ] || continue
      echo "  - $model"
      echo "    install: ollama pull $model"
    done <<EOF
$missing_models
EOF
  else
    echo "[OK] Every configured Ollama model is installed"
  fi
fi

if [ "$check_only" = true ]; then
  if [ "$deployment_ready" = true ]; then
    echo "Deployment prerequisites are ready."
    exit 0
  fi
  echo "Deployment prerequisites are incomplete. Install the missing items above."
  exit 1
fi

if [ "$build_prerequisites_ready" = false ]; then
  echo "Cannot build until the missing build prerequisites are installed." >&2
  exit 1
fi

if [ "$frontend_present" = true ]; then
  if [ "$frontend_prerequisites_ready" = false ]; then
    echo "Cannot install frontend dependencies until Node.js and npm are installed." >&2
  else
    if [ ! -f "$frontend_dir/.env" ] && [ -f "$frontend_dir/.env.example" ]; then
      cp "$frontend_dir/.env.example" "$frontend_dir/.env"
      echo "[OK] Created frontend/.env from frontend/.env.example"
    fi

    echo "Installing frontend dependencies"
    if [ -f "$frontend_dir/package-lock.json" ]; then
      (cd "$frontend_dir" && npm ci)
    else
      (cd "$frontend_dir" && npm install)
    fi
  fi
fi

mkdir -p data build
cmake -S . -B build -DEDU_AI_ENABLE_DROGON=ON
cmake --build build
./build/edu_ai --init-db "${EDU_AI_DATABASE_PATH}"

if [ "$deployment_ready" = true ]; then
  echo "Database, application core, frontend dependencies, and local AI model pool are ready."
else
  echo "Database and application core are ready, but deployment is incomplete. Install the items listed above." >&2
  exit 1
fi
