#!/usr/bin/env sh
set -eu

[ -f .env ] || cp .env.example .env
mkdir -p data build
cmake -S . -B build
cmake --build build
set -a
. ./.env
set +a
./build/edu_ai --init-db "${EDU_AI_DATABASE_PATH}"
echo "Database and application core are ready. Configure provider settings before running chat."
