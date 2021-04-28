#pragma once

#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <GL/gl.h>

#include <string>

class Renderer {
    public:
        Renderer();
        void set_constant( const std::string szConstName, float x );
        void set_constant( const std::string szConstName, float x, float y );
        int reload(std::string shader_src);
        void draw();

    private:
        GLint program;
};
