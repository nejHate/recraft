#pragma once

#include <SDL2/SDL.h>
#include <string>

class Display{
    public:
        Display(int width, int height, const std::string& title);

        void Update();
        bool isClosed();

        virtual ~Display();
    
    protected:
    private:
        Display(const Display& other) {}
        Display& operator=(const Display& other) {}

        SDL_Window* m_window;
        SDL_GLContext m_glContext;
        bool m_isClosed;
};