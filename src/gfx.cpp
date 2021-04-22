#include "gfx.h"

#include <iostream>

Renderer::Renderer(uint32_t width, uint32_t height, uint32_t fps) : frame_period(1000 / fps) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    this->window = SDL_CreateWindow("", 0, 0, width, height, SDL_WINDOW_OPENGL);
    SDL_GL_CreateContext(this->window);
    SDL_ShowCursor(false);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "glewInit failed: " << glewGetErrorString(err) << std::endl;
        exit(-2);
    }

    lastframe = SDL_GetTicks();
}

void Renderer::draw() {
    if (SDL_GetTicks() - lastframe > this->frame_period) {
        glRecti(-1, -1, 1, 1);
        SDL_GL_SwapWindow(this->window);
        lastframe = SDL_GetTicks();
    }
}

void Shader::set_constant(const std::string const_name, float x) {
    const GLchar *const_name_gl = const_name.c_str();

    GLint location = glGetUniformLocation(this->program, const_name_gl);
    if (location != -1) {
        glProgramUniform1f(this->program, location, x);
    }
}

void Shader::set_constant(const std::string const_name, float x, float y) {
    const GLchar *const_name_gl = const_name.c_str();

    GLint location = glGetUniformLocation(this->program, const_name_gl);
    if (location != -1) {
        glProgramUniform2f(this->program, location, x, y);
    }
}

int Shader::reload(std::string shader_src) {
    // compile shader
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *frag_source_gl = shader_src.c_str();
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

        return -1;
    }

    glUseProgram(p);

    this->program = p;
    return 0;
}
