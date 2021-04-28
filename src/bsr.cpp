#define GL_GLEXT_PROTOTYPES

#include "renderer.h"

#include <SDL2/SDL.h>
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
#define SCREEN_PERIOD 1000 / SCREEN_FPS

std::string read_file(std::string filename) {
    std::ifstream in(filename);
    std::ostringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " shader_file" << std::endl;
        return -1;
    }
    char *shader_path = argv[1];

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window = SDL_CreateWindow("", 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GL_CreateContext(window);
    SDL_ShowCursor(false);

    Renderer renderer;
    std::string shader_src = read_file(shader_path);
    renderer.reload(shader_src);
    renderer.set_constant("v2Resolution", CANVAS_WIDTH, CANVAS_HEIGHT);

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
    uint8_t *event_buf = (uint8_t *)malloc(EVENT_BUF_SIZE + 1);
    uint32_t lastframe = SDL_GetTicks();
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
                    std::string shader_src = read_file(shader_path);
                    renderer.reload(shader_src);
                    renderer.set_constant("v2Resolution", CANVAS_WIDTH, CANVAS_HEIGHT);
                }
                event_buf_i += sizeof(struct inotify_event) + event->len;
            }
        }

        if (!SDL_PollEvent(&Event)) {
            if (Event.type == SDL_QUIT || (Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }
        }

        renderer.set_constant("fGlobalTime", SDL_GetTicks()/1000.0);

        if (SDL_GetTicks() - lastframe > SCREEN_PERIOD) {
            renderer.draw();
            SDL_GL_SwapWindow(window);
            lastframe = SDL_GetTicks();
        }
    }

    inotify_rm_watch(inotfd, watchfd);
    return 0;
}
