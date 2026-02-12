#include "failures/failure_injector.h"

namespace dss {

EventId FailureInjector::schedule_crash(TimeMs at, NodeId node) {
  return engine_.schedule(at, node, "failure:crash", [this, node]() { crash(node); });
}

EventId FailureInjector::schedule_recovery(TimeMs at, NodeId node) {
  return engine_.schedule(at, node, "failure:recover", [this, node]() { recover(node); });
}

void FailureInjector::crash(NodeId node) {
  alive_.erase(node);
  auto it = node_events_.find(node);
  if (it != node_events_.end()) {
    for (EventId id : it->second) {
      engine_.cancel(id);
    }
  }
  history_.push_back({engine_.now(), "crash:" + std::to_string(node)});
}

void FailureInjector::recover(NodeId node) {
  alive_.insert(node);
  history_.push_back({engine_.now(), "recover:" + std::to_string(node)});
}

bool FailureInjector::is_alive(NodeId node) const { return alive_.count(node) != 0U; }

void FailureInjector::register_event(NodeId node, EventId event_id) {
  node_events_[node].push_back(event_id);
}

}  // namespace dss
