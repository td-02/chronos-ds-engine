#pragma once

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "core/event.h"

namespace dss {

struct MetricsSnapshot {
  std::map<std::string, std::uint64_t> counters;
  std::vector<std::pair<TimeMs, std::string>> timeline;
};

class MetricsCollector {
 public:
  void inc(const std::string& key, std::uint64_t delta = 1) { counters_[key] += delta; }

  void mark(TimeMs t, const std::string& name) { timeline_.push_back({t, name}); }

  [[nodiscard]] std::uint64_t get(const std::string& key) const {
    auto it = counters_.find(key);
    return it == counters_.end() ? 0 : it->second;
  }

  void to_csv(const std::string& path) const;

  [[nodiscard]] MetricsSnapshot snapshot() const { return {counters_, timeline_}; }
  void restore(const MetricsSnapshot& s) {
    counters_ = s.counters;
    timeline_ = s.timeline;
  }

 private:
  std::map<std::string, std::uint64_t> counters_;
  std::vector<std::pair<TimeMs, std::string>> timeline_;
};

}  // namespace dss
