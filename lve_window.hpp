#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace lve{

  class LveWindow{

    public:
      LveWindow(int w, int h, std::string name);
      ~LveWindow();

<<<<<<< HEAD
      bool shouldClose() { return glfwWindowShouldClose(window); }    

=======
>>>>>>> 9db83ae6a3e1fec4bdd877fb0ba5338e6021debd
    private:
      void initWindow();

      const int width;
      const int height;

      std::string windowName;
      GLFWwindow *window;
  };
}