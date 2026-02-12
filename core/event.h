#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace dss {

using NodeId = std::uint32_t;
using EventId = std::uint64_t;
using TimeMs = std::uint64_t;

struct Event {
  TimeMs timestamp{0};
  NodeId node_id{0};
  EventId event_id{0};
  std::string label;
  std::function<void()> action;

  bool operator<(const Event& other) const {
    if (timestamp != other.timestamp) {
      return timestamp < other.timestamp;
    }
    if (node_id != other.node_id) {
      return node_id < other.node_id;
    }
    return event_id < other.event_id;
  }
};

}  // namespace dss
