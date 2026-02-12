#pragma once

#include <map>
#include <string>
#include <vector>

#include "algorithms/distributed_algorithm.h"

namespace dss {

enum class RaftRole { Follower, Candidate, Leader };

struct LogEntry {
  std::uint64_t term{0};
  std::string command;
};

struct RaftNodeState {
  NodeId self{0};
  std::vector<NodeId> peers;
  RaftRole role{RaftRole::Follower};
  std::uint64_t current_term{0};
  NodeId voted_for{0};
  std::vector<LogEntry> log;
  std::uint64_t commit_index{0};
  std::uint64_t votes_received{0};
  TimeMs election_timeout_ms{0};
  TimeMs heartbeat_ms{0};
  std::uint64_t leader_changes{0};
};

class SimplifiedRaft : public DistributedAlgorithm {
 public:
  explicit SimplifiedRaft(RaftNodeState initial) : state_(std::move(initial)) {}

  void on_start(AlgorithmContext& ctx, NodeId self) override;
  void on_message(AlgorithmContext& ctx, NodeId self, const Message& m) override;
  void on_timeout(AlgorithmContext& ctx, NodeId self,
                  const std::string& timeout_name) override;

  [[nodiscard]] const RaftNodeState& state() const { return state_; }

 private:
  void reset_election(AlgorithmContext& ctx);
  void broadcast(AlgorithmContext& ctx, const std::string& type,
                 const std::string& payload);
  void become_leader(AlgorithmContext& ctx);

  RaftNodeState state_;
};

}  // namespace dss
