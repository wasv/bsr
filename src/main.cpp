#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <GL/gl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <iostream>
#include <stdio.h>

#include "frag.h"

#define CANVAS_WIDTH 1920
#define CANVAS_HEIGHT 1080

static void on_render() { glRecti(-1, -1, 1, 1); }

static int on_update() {
    // compile shader
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *frag_source_gl = frag_source.c_str();
    glShaderSource(f, 1, &frag_source_gl, NULL);
    glCompileShader(f);

    GLint isCompiled = 0;
    glGetShaderiv(f, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(f, GL_INFO_LOG_LENGTH, &maxLength);

        char *error = (char *)malloc(maxLength);
        glGetShaderInfoLog(f, maxLength, &maxLength, error);
        std::cerr << error << std::endl;

        return -1;
    }

    // link shader
    GLuint p = glCreateProgram();
    glAttachShader(p, f);
    glLinkProgram(p);

    GLint isLinked = 0;
    glGetProgramiv(p, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &maxLength);

        char *error = (char *)malloc(maxLength);
        glGetProgramInfoLog(p, maxLength, &maxLength, error);
        std::cerr << error << std::endl;

        return -2;
    }

    glUseProgram(p);
    return 0;
}

static void on_realize() {
    glewExperimental = GL_TRUE;
    glewInit();
    on_update();
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window *mainwindow = SDL_CreateWindow("", 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GL_CreateContext(mainwindow);
    SDL_ShowCursor(false);
    on_realize();

    while (true) {
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {
            if (Event.type == SDL_QUIT || (Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_ESCAPE)) {
                return 0;
            }
            if (Event.type == SDL_WINDOWEVENT && Event.window.event == SDL_WINDOWEVENT_EXPOSED) {
                on_render();
                SDL_GL_SwapWindow(mainwindow);
            }
        }
    }

    return 0;
}
