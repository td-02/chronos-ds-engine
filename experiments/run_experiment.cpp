#include <iostream>

#include "experiments/config.h"
#include "experiments/simulator.h"

int main(int argc, char** argv) {
  const std::string config_path = argc > 1 ? argv[1] : "configs/example_experiment.json";

  dss::ExperimentConfig cfg = dss::load_json_file(config_path);
  dss::DeterministicSimulator sim(cfg);
  sim.initialize();
  sim.run_until(100);
  sim.issue_client_command(1, "set x=1");
  sim.run_all();
  sim.write_metrics(cfg.output_csv);

  std::cout << "Wrote metrics to " << cfg.output_csv << "\n";
  return 0;
}
