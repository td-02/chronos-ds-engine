#pragma once

#include <cstdint>
#include <string>

namespace dss {

struct ExperimentConfig {
  std::uint32_t seed{7};
  std::uint32_t nodes{5};
  std::uint64_t duration_ms{400};
  std::uint64_t fixed_delay_ms{8};
  std::uint64_t jitter_ms{4};
  double loss_rate{0.0};
  double reorder_rate{0.15};
  std::uint64_t crash_at_ms{120};
  std::uint32_t crash_node{2};
  std::string output_csv{"experiments/output.csv"};
};

ExperimentConfig parse_json_config(const std::string& json_text);
ExperimentConfig load_json_file(const std::string& path);

}  // namespace dss
