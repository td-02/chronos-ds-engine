#!/usr/bin/env bash
set -euo pipefail

CONFIG="${1:-configs/example_experiment.json}"
OUT=$(python3 - <<'PY' "$CONFIG"
import json,sys
print(json.load(open(sys.argv[1]))['output_csv'])
PY
)

./build/run_experiment "$CONFIG"
cp "$OUT" experiments/run1.csv
./build/run_experiment "$CONFIG"
cp "$OUT" experiments/run2.csv

if diff -u experiments/run1.csv experiments/run2.csv; then
  echo "Deterministic: identical outputs"
else
  echo "Non-deterministic: outputs differ"
  exit 1
fi
