#include "core/event_engine.h"

#include <stdexcept>
#include <tuple>

namespace dss {

EventId EventEngine::schedule(TimeMs timestamp, NodeId node_id, std::string label,
                              std::function<void()> action) {
  Event e{timestamp, node_id, next_event_id_, std::move(label), std::move(action)};
  const auto key = std::make_tuple(e.timestamp, e.node_id, e.event_id);
  queue_[key] = e.event_id;
  queue_by_id_[e.event_id] = std::move(e);
  return next_event_id_++;
}

void EventEngine::cancel(EventId id) { cancelled_.insert(id); }

bool EventEngine::run_next() {
  if (queue_.empty()) {
    return false;
  }
  auto it = queue_.begin();
  EventId id = it->second;
  queue_.erase(it);

  auto qit = queue_by_id_.find(id);
  if (qit == queue_by_id_.end()) {
    throw std::runtime_error("Internal scheduler inconsistency");
  }
  Event event = qit->second;
  queue_by_id_.erase(qit);

  if (cancelled_.count(id) != 0U) {
    return true;
  }

  now_ = event.timestamp;
  executed_.push_back({event.timestamp, event.node_id, event.event_id, event.label});
  event.action();
  return true;
}

void EventEngine::run_until(TimeMs end_time) {
  while (!queue_.empty()) {
    auto key = queue_.begin()->first;
    if (std::get<0>(key) > end_time) {
      break;
    }
    run_next();
  }
}

void EventEngine::run_all() {
  while (run_next()) {
  }
}

EventEngineSnapshot EventEngine::snapshot() const {
  EventEngineSnapshot s;
  s.now = now_;
  s.next_event_id = next_event_id_;
  s.executed = executed_;
  s.cancelled = cancelled_;
  for (const auto& [id, e] : queue_by_id_) {
    s.queued[id] = {e.timestamp, e.node_id, e.event_id, e.label};
  }
  return s;
}

void EventEngine::restore(const EventEngineSnapshot& state,
                          const std::map<EventId, std::function<void()>>& callbacks) {
  now_ = state.now;
  next_event_id_ = state.next_event_id;
  executed_ = state.executed;
  cancelled_ = state.cancelled;
  queue_.clear();
  queue_by_id_.clear();

  for (const auto& [id, se] : state.queued) {
    auto it = callbacks.find(id);
    if (it == callbacks.end()) {
      throw std::runtime_error("Missing callback during restore for event id " +
                               std::to_string(id));
    }
    Event e{se.timestamp, se.node_id, se.event_id, se.label, it->second};
    queue_by_id_[id] = e;
    queue_[std::make_tuple(e.timestamp, e.node_id, e.event_id)] = id;
  }
}

}  // namespace dss
