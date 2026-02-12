#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "core/event_engine.h"

namespace dss {

class FailureInjector {
 public:
  explicit FailureInjector(EventEngine& engine) : engine_(engine) {}

  EventId schedule_crash(TimeMs at, NodeId node);
  EventId schedule_recovery(TimeMs at, NodeId node);

  void crash(NodeId node);
  void recover(NodeId node);

  [[nodiscard]] bool is_alive(NodeId node) const;
  void register_event(NodeId node, EventId event_id);

  [[nodiscard]] const std::vector<std::pair<TimeMs, std::string>>& history() const {
    return history_;
  }

 private:
  EventEngine& engine_;
  std::set<NodeId> alive_;
  std::map<NodeId, std::vector<EventId>> node_events_;
  std::vector<std::pair<TimeMs, std::string>> history_;
};

}  // namespace dss
