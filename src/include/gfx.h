#pragma once

#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <GL/gl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_timer.h>

#include <string>

class Shader {
    public:
        void set_constant( const std::string szConstName, float x );
        void set_constant( const std::string szConstName, float x, float y );
        int reload(std::string shader_src);

    private:
        GLint program;
};

class Renderer {
    public:
        Renderer(uint32_t width, uint32_t height, uint32_t fps);
        void draw();

    private:
        SDL_Window *window;
        uint32_t lastframe;
        uint32_t frame_period;
};
