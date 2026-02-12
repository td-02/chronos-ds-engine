#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "core/event.h"

namespace dss {

struct ScheduledEvent {
  bool operator==(const ScheduledEvent& other) const {
    return timestamp == other.timestamp && node_id == other.node_id &&
           event_id == other.event_id && label == other.label;
  }

  TimeMs timestamp{0};
  NodeId node_id{0};
  EventId event_id{0};
  std::string label;
};

struct EventEngineSnapshot {
  TimeMs now{0};
  EventId next_event_id{1};
  std::map<EventId, ScheduledEvent> queued;
  std::vector<ScheduledEvent> executed;
  std::set<EventId> cancelled;
};

class EventEngine {
 public:
  EventEngine() = default;

  EventId schedule(TimeMs timestamp, NodeId node_id, std::string label,
                   std::function<void()> action);

  void cancel(EventId id);

  bool run_next();
  void run_until(TimeMs end_time);
  void run_all();

  [[nodiscard]] TimeMs now() const { return now_; }
  [[nodiscard]] const std::vector<ScheduledEvent>& executed_events() const {
    return executed_;
  }
  [[nodiscard]] bool empty() const { return queue_.empty(); }

  [[nodiscard]] EventEngineSnapshot snapshot() const;
  void restore(const EventEngineSnapshot& state,
               const std::map<EventId, std::function<void()>>& callbacks);

 private:
  TimeMs now_{0};
  EventId next_event_id_{1};
  std::map<EventId, Event> queue_by_id_;
  std::map<std::tuple<TimeMs, NodeId, EventId>, EventId> queue_;
  std::vector<ScheduledEvent> executed_;
  std::set<EventId> cancelled_;
};

}  // namespace dss
