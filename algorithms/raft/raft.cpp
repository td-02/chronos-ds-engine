#include "algorithms/raft/raft.h"

#include <sstream>

namespace dss {

void SimplifiedRaft::on_start(AlgorithmContext& ctx, NodeId /*self*/) { reset_election(ctx); }

void SimplifiedRaft::reset_election(AlgorithmContext& ctx) {
  ctx.schedule_timeout(state_.self, "raft:election", state_.election_timeout_ms);
}

void SimplifiedRaft::broadcast(AlgorithmContext& ctx, const std::string& type,
                               const std::string& payload) {
  for (NodeId peer : state_.peers) {
    if (peer == state_.self) {
      continue;
    }
    Message m;
    m.from = state_.self;
    m.to = peer;
    m.type = type;
    m.payload = payload;
    ctx.send(m);
  }
}

void SimplifiedRaft::become_leader(AlgorithmContext& ctx) {
  state_.role = RaftRole::Leader;
  state_.leader_changes++;
  broadcast(ctx, "append_entries", std::to_string(state_.current_term));
  ctx.schedule_timeout(state_.self, "raft:heartbeat", state_.heartbeat_ms);
}

void SimplifiedRaft::on_message(AlgorithmContext& ctx, NodeId /*self*/, const Message& m) {
  if (m.type == "request_vote") {
    std::uint64_t term = std::stoull(m.payload);
    if (term > state_.current_term) {
      state_.current_term = term;
      state_.role = RaftRole::Follower;
      state_.voted_for = 0;
    }

    if (term == state_.current_term && (state_.voted_for == 0 || state_.voted_for == m.from)) {
      state_.voted_for = m.from;
      Message reply{state_.self, m.from, "vote_granted", std::to_string(state_.current_term)};
      ctx.send(reply);
      reset_election(ctx);
    }
    return;
  }

  if (m.type == "vote_granted" && state_.role == RaftRole::Candidate) {
    std::uint64_t term = std::stoull(m.payload);
    if (term == state_.current_term) {
      ++state_.votes_received;
      if (state_.votes_received > (state_.peers.size() / 2)) {
        become_leader(ctx);
      }
    }
    return;
  }

  if (m.type == "append_entries") {
    std::uint64_t term = std::stoull(m.payload);
    if (term >= state_.current_term) {
      if (term > state_.current_term || state_.role != RaftRole::Follower) {
        state_.leader_changes++;
      }
      state_.current_term = term;
      state_.role = RaftRole::Follower;
      state_.voted_for = 0;
      reset_election(ctx);
    }
    return;
  }

  if (m.type == "client_command" && state_.role == RaftRole::Leader) {
    state_.log.push_back({state_.current_term, m.payload});
    state_.commit_index = state_.log.size();
    broadcast(ctx, "replicate", m.payload + "@" + std::to_string(state_.current_term));
    return;
  }

  if (m.type == "replicate") {
    auto at = m.payload.rfind('@');
    if (at != std::string::npos) {
      std::string cmd = m.payload.substr(0, at);
      std::uint64_t term = std::stoull(m.payload.substr(at + 1));
      if (term >= state_.current_term) {
        state_.current_term = term;
        state_.log.push_back({term, cmd});
        state_.commit_index = state_.log.size();
      }
    }
  }
}

void SimplifiedRaft::on_timeout(AlgorithmContext& ctx, NodeId /*self*/,
                                const std::string& timeout_name) {
  if (timeout_name == "raft:election") {
    if (state_.role == RaftRole::Leader) {
      return;
    }
    state_.role = RaftRole::Candidate;
    state_.current_term++;
    state_.voted_for = state_.self;
    state_.votes_received = 1;
    broadcast(ctx, "request_vote", std::to_string(state_.current_term));
    reset_election(ctx);
    return;
  }

  if (timeout_name == "raft:heartbeat" && state_.role == RaftRole::Leader) {
    broadcast(ctx, "append_entries", std::to_string(state_.current_term));
    ctx.schedule_timeout(state_.self, "raft:heartbeat", state_.heartbeat_ms);
  }
}

}  // namespace dss
