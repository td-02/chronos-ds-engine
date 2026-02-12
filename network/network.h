#pragma once

#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "clock/logical_clock.h"
#include "core/event_engine.h"

namespace dss {

struct Message {
  NodeId from{0};
  NodeId to{0};
  std::string type;
  std::string payload;
  std::uint64_t lamport{0};
  std::map<NodeId, std::uint64_t> vector;
};

struct NetworkSnapshot {
  std::mt19937 rng;
  std::map<std::pair<NodeId, NodeId>, bool> partitions;
  std::vector<EventId> in_flight;
};

class Network {
 public:
  struct Config {
    TimeMs fixed_delay_ms{10};
    TimeMs jitter_ms{0};
    double packet_loss_rate{0.0};
    double reorder_rate{0.0};
    std::uint32_t seed{1};
  };

  explicit Network(EventEngine& engine, Config cfg)
      : engine_(engine), cfg_(cfg), rng_(cfg.seed) {}

  void register_nodes(const std::vector<NodeId>& nodes);

  void set_partition(NodeId a, NodeId b, bool blocked);

  EventId send(const Message& message,
               const std::function<void(const Message&)>& deliver);

  [[nodiscard]] NetworkSnapshot snapshot() const;
  void restore(const NetworkSnapshot& snapshot);

 private:
  bool is_partitioned(NodeId a, NodeId b) const;

  EventEngine& engine_;
  Config cfg_;
  std::mt19937 rng_;
  std::uniform_real_distribution<double> real_dist_{0.0, 1.0};
  std::uniform_int_distribution<TimeMs> jitter_dist_{0, 0};
  std::map<std::pair<NodeId, NodeId>, bool> partitions_;
  std::vector<EventId> in_flight_;
};

}  // namespace dss
