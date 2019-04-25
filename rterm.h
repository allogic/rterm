#ifndef RTERM_H
#define RTERM_H

#include <iostream>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <thread>
#include <chrono>

extern "C" {
#include <linux/input.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
}

#define CSI "\033["
#define CLS CSI "2J"
#define HCS CSI "?25l"
#define SCS CSI "?25h"

#define MOV(x, y) CSI + std::to_string(y + 1) + ";" + std::to_string(x + 1) + "H"

namespace rtt {
  struct context {
    public:
      char * buffer;
      struct input_event * event;

      context(int size) {
        buffer = new char[size];
        std::memset(buffer, 0, sizeof(char) * size);

        event = new input_event();
        std::memset(event, 0, sizeof(input_event));
      }
      ~context() {
        delete[] buffer;

        free(event);
      }

      char & operator [] (int i) { return buffer[i]; }
  };
  static struct termios ptty, ctty;

  static bool (*hook_start)();
  static bool (*hook_update)();

  static int fd = -1, t = 0, w = 100, h = 50;

  static context * ctx;

  static bool init_keyboard(const char * device) {
    if ((fd = open(device, O_RDONLY | O_SYNC)) == -1) {
      std::cout << "can not open keyboard fd " << std::endl;

      return false;
    }

    int flags = fcntl(fd, F_GETFL, 0);

    flags |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1) {
      std::cout << "can not set keyboard fd flags" << std::endl;

      return false;
    }

    return true;
  }

  static bool restore_tty() {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &ptty) != 0) {
      std::cout << "can not restore default tty attr" << std::endl;

      return false;
    }

    return true;
  }

  static bool init_tty() {
    std::memset(&ctty, 0, sizeof(ctty));

    if (tcgetattr(STDIN_FILENO, &ctty) != 0) {
      std::cout << "can not get tty attr" << std::endl;

      return false;
    }

    std::memcpy(&ptty, &ctty, sizeof(ptty));

    ctty.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &ctty) != 0) {
      std::cout << "can not set tty attr" << std::endl;

      return false;
    }

    return true;
  }

  static void restore() {
    delete ctx;

    std::cout << "\r" SCS;

    close(fd);

    if (!restore_tty())
      std::cout << "can not restore tty defaults" << std::endl;
  }

  static bool init(const char * device, int argc, char ** argv) {
    std::string::size_type st;

    if (argc >= 2) t = std::stoi(argv[1], &st);
    if (argc >= 3) w = std::stoi(argv[2], &st);
    if (argc >= 4) h = std::stoi(argv[3], &st);

    if (!init_tty()) return false;
    if (!init_keyboard(device)) return false;

    std::signal(SIGINT, [](int s) { restore(); exit(s); });

    std::cout << CLS MOV(0, 0) HCS;

    return true;
  }

  static void start() {
    ctx = new context(w * h);

    if (!hook_start()) {
      restore();

      return;
    }

    for(;;) {
      read(fd, ctx->event, sizeof(input_event));

      if (!hook_update()) {
        restore();

        return;
      }

      for (int y = 0; y < h; y++) {
        std::cout << MOV(0, y);

        for (int x = 0; x < w; x++)
          std::cout << ctx->buffer[x + y * w];
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(t));
    }
  }
}

#endif
