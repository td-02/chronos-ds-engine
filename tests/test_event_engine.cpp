#include <cassert>
#include <string>
#include <vector>

#include "core/event_engine.h"

int main() {
  dss::EventEngine engine;
  std::vector<std::string> trace;

  engine.schedule(10, 2, "B", [&]() { trace.push_back("B"); });
  engine.schedule(10, 1, "A", [&]() { trace.push_back("A"); });
  engine.schedule(10, 1, "C", [&]() { trace.push_back("C"); });

  engine.run_all();

  assert((trace == std::vector<std::string>{"A", "C", "B"}));
  assert(engine.executed_events().size() == 3);

  dss::EventEngine e1;
  dss::EventEngine e2;
  std::string s1;
  std::string s2;

  for (int i = 0; i < 20; ++i) {
    e1.schedule(i % 5, i % 3, "x", [&]() { s1 += "x"; });
    e2.schedule(i % 5, i % 3, "x", [&]() { s2 += "x"; });
  }

  e1.run_all();
  e2.run_all();

  assert(s1 == s2);
  assert(e1.executed_events() == e2.executed_events());
  return 0;
}
