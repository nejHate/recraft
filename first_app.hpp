#pragma once

#include "lve_window.cpp"

namespace lve {
  class FirstApp {
    public:
      static constexpr int WIDTH = 800;
      static constexpr int HEIGHT = 600;
<<<<<<< HEAD

      void run();

=======
      void run() {
        return;
      };
>>>>>>> 9db83ae6a3e1fec4bdd877fb0ba5338e6021debd
    private:
      LveWindow lveWindow{WIDTH, HEIGHT, "Hello Wulkan!"};
  };
}