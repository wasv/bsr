#include <iostream>

#include "renderer.h"

Renderer::Renderer() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK && err != 4) {
        std::cerr << "glewInit failed: " << glewGetErrorString(err) << " " << err << std::endl;
        exit(-2);
    }
}

void Renderer::draw() {
    glRecti(-1, -1, 1, 1);
}

void Renderer::set_constant(const std::string const_name, float x) {
    const GLchar *const_name_gl = const_name.c_str();

    GLint location = glGetUniformLocation(this->program, const_name_gl);
    if (location != -1) {
        glProgramUniform1f(this->program, location, x);
    }
}

void Renderer::set_constant(const std::string const_name, float x, float y) {
    const GLchar *const_name_gl = const_name.c_str();

    GLint location = glGetUniformLocation(this->program, const_name_gl);
    if (location != -1) {
        glProgramUniform2f(this->program, location, x, y);
    }
}

int Renderer::reload(std::string shader_src) {
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
