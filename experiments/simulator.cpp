#include "experiments/simulator.h"

#include <sstream>

namespace dss {

DeterministicSimulator::DeterministicSimulator(const ExperimentConfig& cfg)
    : network_(engine_,
               Network::Config{cfg.fixed_delay_ms, cfg.jitter_ms, cfg.loss_rate,
                               cfg.reorder_rate, cfg.seed}),
      failures_(engine_),
      cfg_(cfg) {}

void DeterministicSimulator::initialize() {
  std::vector<NodeId> nodes;
  for (NodeId i = 1; i <= cfg_.nodes; ++i) {
    nodes.push_back(i);
  }
  network_.register_nodes(nodes);

  for (NodeId id : nodes) {
    RaftNodeState state;
    state.self = id;
    state.peers = nodes;
    state.election_timeout_ms = 50 + id * 10;
    state.heartbeat_ms = 20;
    raft_[id] = std::make_unique<SimplifiedRaft>(state);
    lamport_[id] = LamportClock{};
    vector_.emplace(id, VectorClock(nodes));
    EventId rec = failures_.schedule_recovery(0, id);
    callback_registry_[rec] = [this, id]() { failures_.recover(id); };
  }

  for (NodeId id : nodes) {
    auto cb = [this, id]() { raft_.at(id)->on_start(*this, id); };
    EventId eid = engine_.schedule(0, id, "node:start", cb);
    callback_registry_[eid] = cb;
  }

  EventId crash = failures_.schedule_crash(cfg_.crash_at_ms, cfg_.crash_node);
  callback_registry_[crash] = [this]() { failures_.crash(cfg_.crash_node); };
}

void DeterministicSimulator::run_until(TimeMs t) { engine_.run_until(t); }

void DeterministicSimulator::run_all() { engine_.run_until(cfg_.duration_ms); }

void DeterministicSimulator::issue_client_command(NodeId leader_hint,
                                                  const std::string& command) {
  Message m;
  m.from = 0;
  m.to = leader_hint;
  m.type = "client_command";
  m.payload = command;
  send(m);
}

void DeterministicSimulator::write_metrics(const std::string& path) const {
  metrics_.to_csv(path);
}

void DeterministicSimulator::send(const Message& m) {
  if (m.to != 0 && !failures_.is_alive(m.to)) {
    return;
  }

  Message enriched = m;
  if (m.from != 0) {
    enriched.lamport = lamport_.at(m.from).tick();
    vector_.at(m.from).tick(m.from);
    enriched.vector = vector_.at(m.from).values();
  }

  auto deliver_cb = [this](const Message& incoming) {
    if (incoming.to != 0 && !failures_.is_alive(incoming.to)) {
      return;
    }

    if (incoming.to != 0 && incoming.from != 0) {
      lamport_.at(incoming.to).on_receive(incoming.lamport);
      vector_.at(incoming.to).on_receive(incoming.to, incoming.vector);
    }

    metrics_.inc("messages");
    raft_.at(incoming.to)->on_message(*this, incoming.to, incoming);
  };

  EventId e = network_.send(enriched, deliver_cb);
  if (e != 0) {
    failures_.register_event(enriched.to, e);
    callback_registry_[e] = [deliver_cb, enriched]() { deliver_cb(enriched); };
  }
}

void DeterministicSimulator::schedule_timeout(NodeId node, std::string name,
                                              TimeMs delay) {
  auto cb = [this, node, name]() {
    if (!failures_.is_alive(node)) {
      return;
    }
    raft_.at(node)->on_timeout(*this, node, name);
  };

  EventId e = engine_.schedule(engine_.now() + delay, node, name, cb);
  failures_.register_event(node, e);
  callback_registry_[e] = cb;
}

std::map<NodeId, RaftNodeState> DeterministicSimulator::states() const {
  std::map<NodeId, RaftNodeState> out;
  for (const auto& [id, node] : raft_) {
    out[id] = node->state();
  }
  return out;
}

SimulatorSnapshot DeterministicSimulator::snapshot() const {
  SimulatorSnapshot s;
  s.engine = engine_.snapshot();
  s.network = network_.snapshot();
  s.metrics = metrics_.snapshot();
  s.lamports = lamport_;
  for (const auto& [id, vc] : vector_) {
    s.vectors[id] = vc.values();
  }
  for (const auto& [id, node] : raft_) {
    s.raft[id] = node->state();
  }
  return s;
}

void DeterministicSimulator::restore(const SimulatorSnapshot& snap) {
  network_.restore(snap.network);
  metrics_.restore(snap.metrics);
  lamport_ = snap.lamports;

  std::vector<NodeId> nodes;
  for (const auto& [id, _] : snap.raft) {
    nodes.push_back(id);
  }

  vector_.clear();
  raft_.clear();
  for (const auto& [id, state] : snap.raft) {
    vector_.emplace(id, VectorClock(nodes));
    vector_.at(id).merge(snap.vectors.at(id));
    raft_[id] = std::make_unique<SimplifiedRaft>(state);
  }

  engine_.restore(snap.engine, callback_registry_);
}

}  // namespace dss
