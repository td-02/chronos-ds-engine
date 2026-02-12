#pragma once

#include <string>

#include "network/network.h"

namespace dss {

class AlgorithmContext {
 public:
  virtual ~AlgorithmContext() = default;
  virtual void send(const Message& m) = 0;
  virtual void schedule_timeout(NodeId node, std::string name, TimeMs delay) = 0;
  virtual TimeMs now() const = 0;
};

class DistributedAlgorithm {
 public:
  virtual ~DistributedAlgorithm() = default;
  virtual void on_start(AlgorithmContext& ctx, NodeId self) = 0;
  virtual void on_message(AlgorithmContext& ctx, NodeId self, const Message& m) = 0;
  virtual void on_timeout(AlgorithmContext& ctx, NodeId self,
                          const std::string& timeout_name) = 0;
};

}  // namespace dss
