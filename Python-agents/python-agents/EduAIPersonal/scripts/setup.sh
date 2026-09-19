#!/usr/bin/env sh
set -eu

[ -f .env ] || cp .env.example .env
mkdir -p data build
cmake -S . -B build
cmake --build build
echo "Skeleton setup complete. Configure .env before adding providers."

