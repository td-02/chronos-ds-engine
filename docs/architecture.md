# Architecture

## Directory and file purposes
- `core/event.h`: event primitives and ordering key types.
- `core/event_engine.h/.cpp`: deterministic scheduler, cancellation, execution trace, snapshot/restore.
- `clock/logical_clock.h`: Lamport and vector clocks, happens-before function.
- `network/network.h/.cpp`: deterministic latency/loss/reorder/partition transport.
- `failures/failure_injector.h/.cpp`: crash/recovery scheduling and cancellation.
- `algorithms/distributed_algorithm.h`: plugin interface with inversion-of-control context.
- `algorithms/raft/raft.h/.cpp`: simplified deterministic Raft behavior.
- `metrics/metrics.h/.cpp`: deterministic counters/timeline CSV output.
- `experiments/config.h/.cpp`: config model and minimal JSON reader.
- `experiments/simulator.h/.cpp`: simulator composition root and snapshot logic.
- `experiments/run_experiment.cpp`: CLI entrypoint.
- `tests/*.cpp`: deterministic property validation.

## Event ordering explanation
Scheduler total order key is `(timestamp, node_id, event_id)`.
- `timestamp` enforces virtual-time progression.
- `node_id` removes tie ambiguity at equal time.
- `event_id` enforces FIFO among same `(time,node)` schedules.

## Happens-before semantics
- Lamport: `L_recv = max(L_local, L_incoming) + 1`.
- Vector: merge pointwise max and increment local component.
- HB relation: `A -> B` iff all vector components `A <= B` and at least one `<`.

## Failure model definition
- Crash/recover events are inserted in deterministic scheduler.
- Crashed node behavior:
  - drop incoming messages
  - ignore timeout callbacks
  - cancel node-owned future events registered via injector

## Safety and liveness (simplified Raft)
- **Safety intent:** at most one leader per term under deterministic timing and no split vote tie loops.
- **Liveness intent:** election timeouts eventually trigger a leader in non-partitioned majority.
- **Limits:** this implementation is educational and intentionally omits complete Raft log matching guarantees.
