# Deterministic Simulation of Distributed Systems: A Reproducible C++17 Architecture

## Abstract
We present a deterministic distributed systems simulator implemented in C++17 for reproducible protocol experimentation. The platform enforces a strict event order, deterministic network model, controlled failure injection, and snapshot/restore for time-travel replay. We demonstrate integration with a simplified Raft protocol and provide deterministic metrics generation suitable for repeatable research workflows.

## Introduction
Distributed systems research routinely suffers from nondeterministic execution artifacts that complicate debugging and reproducibility. To address this, we designed a simulator where all state transitions are driven by a single-threaded virtual-time scheduler and seeded pseudo-randomness.

## Design
Core contributions include: (1) strict event ordering by `(timestamp, node_id, event_id)`, (2) seeded network perturbations (delay/loss/reorder/partition), (3) deterministic failure injection through scheduled crash/recovery events, (4) protocol plugin inversion-of-control, and (5) snapshotting of all mutable components.

## Evaluation
We evaluate deterministic properties through unit/integration tests:
- scheduler ordering and replay stability;
- clock causality and deterministic transport behavior;
- Raft election/re-election under crash;
- snapshot branch replay equivalence.
Repeated experiment executions produce byte-identical CSV outputs under fixed seed and configuration.

## Limitations
The included Raft implementation is intentionally simplified and omits durability, strict log matching proofs, and full production semantics. The JSON parser handles a constrained schema rather than full standard JSON grammar.

## Future Work
Future extensions include Byzantine fault models, pluggable storage layers, larger protocol corpus, symbolic schedule exploration, and machine-checkable invariants integrated with the simulation trace.
