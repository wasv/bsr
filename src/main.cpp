#define GL_GLEXT_PROTOTYPES

#include "gfx.h"
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

    Renderer renderer = Renderer(CANVAS_WIDTH, CANVAS_HEIGHT, SCREEN_FPS);

    Shader shader;
    std::string shader_src = read_file(shader_path);
    shader.reload(shader_src);
    shader.set_constant("v2Resolution", CANVAS_WIDTH, CANVAS_HEIGHT);

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
                    shader.reload(shader_src);
                    shader.set_constant("v2Resolution", CANVAS_WIDTH, CANVAS_HEIGHT);
                }
                event_buf_i += sizeof(struct inotify_event) + event->len;
            }
        }

        if (!SDL_PollEvent(&Event)) {
            if (Event.type == SDL_QUIT || (Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }
        }

        shader.set_constant("fGlobalTime", SDL_GetTicks());
        renderer.draw();
    }

    inotify_rm_watch(inotfd, watchfd);
    return 0;
}
