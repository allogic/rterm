# rterm
Fast and simple realtime ANSI terminal renderer with ray tracing example

## Files
```rterm.h``` - a simple ANSI render engine<br>
```rtrace.h``` -  a basic ray tracing algorithm rendering a signed distance field

## Usage
```rtt::init``` requires a keyboard device name as it's first parameter. You can find your name by simply iterating over all device names with ```sudo cat /dev/input/<name>```, if you get random ASCII characters for every key press then that is the right one.

```cpp
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
```

## Result
![](result.gif)
