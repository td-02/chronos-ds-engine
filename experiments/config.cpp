#include "experiments/config.h"

#include <fstream>
#include <regex>
#include <stdexcept>

namespace dss {
namespace {

template <typename T>
void parse_number(const std::string& text, const std::string& key, T& out) {
  std::regex pattern("\"" + key + "\"\\s*:\\s*([0-9]+(?:\\.[0-9]+)?)");
  std::smatch match;
  if (std::regex_search(text, match, pattern)) {
    if constexpr (std::is_floating_point<T>::value) {
      out = static_cast<T>(std::stod(match[1]));
    } else {
      out = static_cast<T>(std::stoull(match[1]));
    }
  }
}

void parse_string(const std::string& text, const std::string& key, std::string& out) {
  std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
  std::smatch match;
  if (std::regex_search(text, match, pattern)) {
    out = match[1].str();
  }
}

}  // namespace

ExperimentConfig parse_json_config(const std::string& text) {
  ExperimentConfig c;
  parse_number(text, "seed", c.seed);
  parse_number(text, "nodes", c.nodes);
  parse_number(text, "duration_ms", c.duration_ms);
  parse_number(text, "fixed_delay_ms", c.fixed_delay_ms);
  parse_number(text, "jitter_ms", c.jitter_ms);
  parse_number(text, "loss_rate", c.loss_rate);
  parse_number(text, "reorder_rate", c.reorder_rate);
  parse_number(text, "crash_at_ms", c.crash_at_ms);
  parse_number(text, "crash_node", c.crash_node);
  parse_string(text, "output_csv", c.output_csv);
  return c;
}

ExperimentConfig load_json_file(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("Unable to read config file: " + path);
  }
  return parse_json_config(std::string((std::istreambuf_iterator<char>(in)),
                                       std::istreambuf_iterator<char>()));
}

}  // namespace dss
