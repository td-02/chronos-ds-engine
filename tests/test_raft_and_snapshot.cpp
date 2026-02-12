#include <cassert>
#include <sstream>

#include "experiments/simulator.h"

namespace {
std::string summarize(const std::map<dss::NodeId, dss::RaftNodeState>& states) {
  std::ostringstream out;
  for (const auto& [id, st] : states) {
    out << id << ':' << static_cast<int>(st.role) << ':' << st.current_term << ':'
        << st.commit_index << ';';
  }
  return out.str();
}
}  // namespace

int main() {
  dss::ExperimentConfig cfg;
  cfg.nodes = 5;
  cfg.seed = 9;
  cfg.duration_ms = 320;
  cfg.crash_at_ms = 120;
  cfg.crash_node = 2;

  dss::DeterministicSimulator sim(cfg);
  sim.initialize();
  sim.run_until(110);
  sim.issue_client_command(1, "k=v");
  auto snap = sim.snapshot();

  sim.run_all();
  auto final_a = sim.states();

  dss::DeterministicSimulator sim2(cfg);
  sim2.initialize();
  sim2.run_until(110);
  sim2.issue_client_command(1, "k=v");
  sim2.restore(snap);
  sim2.run_all();
  auto final_b = sim2.states();

  assert(summarize(final_a) == summarize(final_b));

  std::size_t leaders = 0;
  for (const auto& [_, st] : final_a) {
    if (st.role == dss::RaftRole::Leader) {
      ++leaders;
    }
  }
  assert(leaders <= 1);

  return 0;
}
