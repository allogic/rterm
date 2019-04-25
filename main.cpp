#include <iostream>

#include "rterm.h"
#include "rtrace.h"

int main(int argc, char ** argv) {
  if (!rtt::init("/dev/input/event3", argc, argv))
    return 0;

  rtt::hook_start = []() { return true; };
  rtt::hook_update = &update;

  rtt::start();

  return 0;
}
