#include <iostream>
#include <GL/glew.h>
#include "display_opengl/display.hpp"

int main(){

    Display display(800, 600, "Hello World!");

    while(!display.isClosed()){
        glClearColor(0.0f, 0.15f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        display.Update();
    }

    std::cout << "SUCCESS END" << std::endl;
}

// sudo apt install libsdl2-dev
// sudo apt install libglew-dev
// sudo apt install libglm-dev