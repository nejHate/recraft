#pragma once

#include "lve_window.cpp"

namespace lve {
  class FirstApp {
    public:
      static constexpr int WIDTH = 800;
      static constexpr int HEIGHT = 600;

      void run();

    private:
      LveWindow lveWindow{WIDTH, HEIGHT, "Hello Wulkan!"};
  };
}