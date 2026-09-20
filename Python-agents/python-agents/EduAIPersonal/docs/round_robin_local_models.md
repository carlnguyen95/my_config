# Local Ollama round-robin benchmark

The local model pool is configured in `config/ai.json`. The router assigns every generation request to the next
model in this fixed order:

1. `gemma4`
2. `gemma4:12b`
3. `gwen3:8b`

The configured tags are passed to Ollama verbatim. Verify that each tag exists with `ollama list` before running a
real benchmark.

## What this phase measures

`RoundRobinProvider` selects a model with an atomic ticket and measures only the downstream provider call. It records
requests, successes, failures, and min/mean/max provider latency for each model. It deliberately does not queue or
throttle requests: a burst of five or ten requests is forwarded immediately so the first measurements expose real
Ollama/GPU contention.

## Run a burst

Build the project, then run a five-request and a ten-request burst:

```sh
./build/ollama_round_robin_benchmark --requests 5 --concurrency 5
./build/ollama_round_robin_benchmark --requests 10 --concurrency 10 --output outputs/round-robin-10.csv
```

The benchmark prints one result per dispatched ticket and a per-model summary. `--output` writes the per-request
records as CSV. Run each shape at least twice: the first run includes model-load effects, while later runs are better
for comparing steady-state latency.

Use `--warmup` when you want one short request sent to each model before the measured burst. This changes the question
from cold-start performance to warm-model performance; keep the setting consistent when comparing results.

## Scope and next decision

Round-robin makes distribution observable but does not protect a model from overlapping requests. After collecting
the 5/10-request data, use the per-model latency and error rate to choose whether the next iteration needs a
per-model concurrency cap and queue.
