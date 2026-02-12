#include <cassert>
#include <vector>

#include "clock/logical_clock.h"
#include "core/event_engine.h"
#include "network/network.h"

int main() {
  dss::LamportClock a;
  dss::LamportClock b;
  auto t1 = a.tick();
  auto t2 = b.on_receive(t1);
  assert(t2 > t1);

  dss::VectorClock va({1, 2});
  dss::VectorClock vb({1, 2});
  va.tick(1);
  vb.on_receive(2, va.values());
  assert(dss::VectorClock::happens_before(va.values(), vb.values()));

  dss::EventEngine engine1;
  dss::EventEngine engine2;
  dss::Network::Config cfg{5, 5, 0.1, 0.5, 42};
  dss::Network n1(engine1, cfg);
  dss::Network n2(engine2, cfg);

  n1.register_nodes({1, 2});
  n2.register_nodes({1, 2});

  std::vector<dss::TimeMs> d1;
  std::vector<dss::TimeMs> d2;

  for (int i = 0; i < 30; ++i) {
    dss::Message m{1, 2, "t", "p", 0, {}};
    n1.send(m, [&](const dss::Message&) { d1.push_back(engine1.now()); });
    n2.send(m, [&](const dss::Message&) { d2.push_back(engine2.now()); });
  }

  engine1.run_all();
  engine2.run_all();

  assert(d1 == d2);
  return 0;
}
