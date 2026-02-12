#pragma once

#include <map>
#include <vector>

#include "core/event.h"

namespace dss {

class LamportClock {
 public:
  std::uint64_t tick() { return ++counter_; }

  std::uint64_t on_receive(std::uint64_t incoming) {
    counter_ = std::max(counter_, incoming) + 1;
    return counter_;
  }

  [[nodiscard]] std::uint64_t value() const { return counter_; }

 private:
  std::uint64_t counter_{0};
};

class VectorClock {
 public:
  explicit VectorClock(std::vector<NodeId> nodes) {
    for (NodeId id : nodes) {
      values_[id] = 0;
    }
  }

  void tick(NodeId self) { ++values_[self]; }

  void merge(const std::map<NodeId, std::uint64_t>& other) {
    for (const auto& [node, value] : other) {
      auto it = values_.find(node);
      if (it == values_.end()) {
        values_[node] = value;
      } else {
        it->second = std::max(it->second, value);
      }
    }
  }

  void on_receive(NodeId self, const std::map<NodeId, std::uint64_t>& other) {
    merge(other);
    tick(self);
  }

  [[nodiscard]] const std::map<NodeId, std::uint64_t>& values() const {
    return values_;
  }

  static bool happens_before(const std::map<NodeId, std::uint64_t>& a,
                             const std::map<NodeId, std::uint64_t>& b) {
    bool strictly_less = false;
    for (const auto& [node, av] : a) {
      auto bit = b.find(node);
      const std::uint64_t bv = bit == b.end() ? 0 : bit->second;
      if (av > bv) {
        return false;
      }
      if (av < bv) {
        strictly_less = true;
      }
    }
    for (const auto& [node, bv] : b) {
      if (a.find(node) == a.end() && bv > 0) {
        strictly_less = true;
      }
    }
    return strictly_less;
  }

 private:
  std::map<NodeId, std::uint64_t> values_;
};

}  // namespace dss
