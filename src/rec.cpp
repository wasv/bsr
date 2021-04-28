#define GL_GLEXT_PROTOTYPES

#include "renderer.h"
#include "encoder.h"

#include <EGL/egl.h>

#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>

#define CANVAS_WIDTH 1920
#define CANVAS_HEIGHT 1080

#define CANVAS_CHANNELS 4
#define CANVAS_NBYTES (CANVAS_CHANNELS * CANVAS_WIDTH * CANVAS_HEIGHT)

#define OUTPUT_FPS 60

static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,  EGL_NONE};

static const EGLint pbufferAttribs[] = {
    EGL_WIDTH, CANVAS_WIDTH, EGL_HEIGHT, CANVAS_HEIGHT, EGL_NONE,
};

std::string read_file(std::string filename) {
    std::ifstream in(filename);
    std::ostringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " shader_file seconds" << std::endl;
        return -1;
    }
    char *shader_path = argv[1];
    uint32_t duration = atof(argv[2]);
    std::ofstream output_file("out.mpeg", std::fstream::binary);

    Encoder encoder(output_file, OUTPUT_FPS, CANVAS_WIDTH, CANVAS_HEIGHT);

    EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDpy == EGL_NO_DISPLAY) {
        std::cerr << "No EGL Display" << std::endl;
        return -2;
    }
    if (eglInitialize(eglDpy, nullptr, nullptr) != EGL_TRUE) {
        std::cerr << "EGL Initialize Failed" << std::endl;
        return -2;
    }

    EGLint numConfigs;
    EGLConfig eglCfg;
    if (eglChooseConfig(eglDpy, nullptr, &eglCfg, 1, &numConfigs) != EGL_TRUE) {
        std::cerr << "EGL Choose Config Failed" << std::endl;
        return -2;
    }
    if (numConfigs == 0) {
        std::cerr << "No EGL Configs" << std::endl;
        return -2;
    }

    EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, pbufferAttribs);
    eglBindAPI(EGL_OPENGL_API);
    EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, NULL);
    if (eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx) != EGL_TRUE) {
        std::cerr << "No EGL Context" << std::endl;
        return -2;
    }

    Renderer renderer;
    std::string shader_src = read_file(shader_path);
    renderer.reload(shader_src);
    renderer.set_constant("v2Resolution", CANVAS_WIDTH, CANVAS_HEIGHT);

    GLubyte *pixels = (GLubyte *)malloc(CANVAS_NBYTES * sizeof(GLubyte));
    for(int i = 0; i<duration*OUTPUT_FPS; i++) {
        renderer.set_constant("fGlobalTime", i*(1.0/OUTPUT_FPS));
        renderer.draw();
        glReadPixels(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        encoder.encode((uint8_t*)pixels);
    }
    encoder.finish();

    eglTerminate(eglDpy);

    free(pixels);
    return 0;
}
