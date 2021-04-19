#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <GL/gl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_timer.h>

#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>

#define CANVAS_WIDTH 1920
#define CANVAS_HEIGHT 1080

#define NUM_EVENTS 5
#define EVENT_SIZE (sizeof(struct inotify_event) + PATH_MAX) /*size of one event*/
#define EVENT_BUF_SIZE (EVENT_SIZE * NUM_EVENTS)

#define SCREEN_FPS 60
#define SCREEN_TICKS_PER_FRAME 1000 / SCREEN_FPS

std::string read_file(std::string filename) {
    std::ifstream in(filename);
    std::ostringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

static int update_shader(std::string shader_src) {
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

        return -2;
    }

    glUseProgram(p);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " shader_file" << std::endl;
        return -1;
    }
    char *shader_path = argv[1];
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window *window = SDL_CreateWindow("", 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GL_CreateContext(window);
    SDL_ShowCursor(false);

    glewExperimental = GL_TRUE;
    glewInit();
    std::string shader = read_file(shader_path);
    update_shader(shader);

    int inotfd = inotify_init1(IN_NONBLOCK);
    if (inotfd < 0) {
        std::cerr << "Could not initialize inotify." << std::endl;
        return -2;
    }

    char *shader_dir = dirname(strdup(shader_path));
    char *shader_file = basename(strdup(shader_path));
    int watchfd = inotify_add_watch(inotfd, shader_dir, IN_MODIFY);
    if (watchfd < 0) {
        std::cerr << "Could not watch file." << std::endl;
        return -2;
    }

    bool quit = false;
    SDL_Event Event;
    uint32_t lastframe = SDL_GetTicks();
    uint8_t *event_buf = (uint8_t *)malloc(EVENT_BUF_SIZE + 1);
    while (!quit) {
        int length = read(inotfd, event_buf, EVENT_BUF_SIZE);
        if (length < 0 && errno != EAGAIN) {
            std::cerr << "Read error on inotify: " << strerror(errno) << std::endl;
        }

        if (length >= 0) {
            size_t event_buf_i = 0;
            while (event_buf_i < length) {
                struct inotify_event *event = (struct inotify_event *)&event_buf[event_buf_i];

                if (strcmp(event->name, shader_file) == 0) {
                    std::cout << event->name << " modified." << std::endl;
                    std::string shader = read_file(shader_path);
                    update_shader(shader);
                }
                event_buf_i += sizeof(struct inotify_event) + event->len;
            }
        }

        if (!SDL_PollEvent(&Event)) {
            if (Event.type == SDL_QUIT || (Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }
        }

        if (SDL_GetTicks() - lastframe > SCREEN_TICKS_PER_FRAME) {
            glRecti(-1, -1, 1, 1);
            SDL_GL_SwapWindow(window);
            lastframe = SDL_GetTicks();
        }
    }

    inotify_rm_watch(inotfd, watchfd);
    return 0;
}
