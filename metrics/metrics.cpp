#include "metrics/metrics.h"

namespace dss {

void MetricsCollector::to_csv(const std::string& path) const {
  std::ofstream out(path);
  out << "kind,key,time,value\n";
  for (const auto& [k, v] : counters_) {
    out << "counter," << k << ",0," << v << "\n";
  }
  for (const auto& [t, n] : timeline_) {
    out << "timeline," << n << ',' << t << ",1\n";
  }
}

}  // namespace dss
