#!/usr/bin/env sh
set -eu
cmake --build build
exec ./build/edu_ai

