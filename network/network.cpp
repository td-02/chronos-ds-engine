#include "network/network.h"

#include <algorithm>

namespace dss {

void Network::register_nodes(const std::vector<NodeId>& nodes) {
  if (cfg_.jitter_ms > 0) {
    jitter_dist_ = std::uniform_int_distribution<TimeMs>(0, cfg_.jitter_ms);
  }

  for (NodeId a : nodes) {
    for (NodeId b : nodes) {
      partitions_[{a, b}] = false;
    }
  }
}

void Network::set_partition(NodeId a, NodeId b, bool blocked) {
  partitions_[{a, b}] = blocked;
  partitions_[{b, a}] = blocked;
}

bool Network::is_partitioned(NodeId a, NodeId b) const {
  auto it = partitions_.find({a, b});
  if (it == partitions_.end()) {
    return false;
  }
  return it->second;
}

EventId Network::send(const Message& message,
                      const std::function<void(const Message&)>& deliver) {
  if (is_partitioned(message.from, message.to)) {
    return 0;
  }

  if (real_dist_(rng_) < cfg_.packet_loss_rate) {
    return 0;
  }

  TimeMs delay = cfg_.fixed_delay_ms + jitter_dist_(rng_);
  if (real_dist_(rng_) < cfg_.reorder_rate) {
    delay += jitter_dist_(rng_);
  }

  const TimeMs deliver_at = engine_.now() + delay;
  EventId id = engine_.schedule(deliver_at, message.to, "net:deliver", [message, deliver]() {
    deliver(message);
  });
  in_flight_.push_back(id);
  return id;
}

NetworkSnapshot Network::snapshot() const {
  return {rng_, partitions_, in_flight_};
}

void Network::restore(const NetworkSnapshot& snapshot) {
  rng_ = snapshot.rng;
  partitions_ = snapshot.partitions;
  in_flight_ = snapshot.in_flight;
}

}  // namespace dss
