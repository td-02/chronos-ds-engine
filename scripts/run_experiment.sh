#!/usr/bin/env bash
set -euo pipefail

cmake -S . -B build
cmake --build build
./build/run_experiment "${1:-configs/example_experiment.json}"
