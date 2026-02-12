#pragma once

#include <memory>

#include "algorithms/raft/raft.h"
#include "clock/logical_clock.h"
#include "experiments/config.h"
#include "failures/failure_injector.h"
#include "metrics/metrics.h"
#include "network/network.h"

namespace dss {

struct SimulatorSnapshot {
  EventEngineSnapshot engine;
  NetworkSnapshot network;
  MetricsSnapshot metrics;
  std::map<NodeId, LamportClock> lamports;
  std::map<NodeId, std::map<NodeId, std::uint64_t>> vectors;
  std::map<NodeId, RaftNodeState> raft;
};

class DeterministicSimulator : public AlgorithmContext {
 public:
  explicit DeterministicSimulator(const ExperimentConfig& cfg);

  void initialize();
  void run_until(TimeMs t);
  void run_all();
  void issue_client_command(NodeId leader_hint, const std::string& command);
  void write_metrics(const std::string& path) const;

  // AlgorithmContext
  void send(const Message& m) override;
  void schedule_timeout(NodeId node, std::string name, TimeMs delay) override;
  [[nodiscard]] TimeMs now() const override { return engine_.now(); }

  [[nodiscard]] std::map<NodeId, RaftNodeState> states() const;

  [[nodiscard]] SimulatorSnapshot snapshot() const;
  void restore(const SimulatorSnapshot& snap);

 private:
  EventEngine engine_;
  Network network_;
  FailureInjector failures_;
  MetricsCollector metrics_;
  ExperimentConfig cfg_;

  std::map<NodeId, std::unique_ptr<SimplifiedRaft>> raft_;
  std::map<NodeId, LamportClock> lamport_;
  std::map<NodeId, VectorClock> vector_;
  std::map<EventId, std::function<void()>> callback_registry_;
};

}  // namespace dss
