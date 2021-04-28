#pragma once

#include <fstream>

extern "C" {
#include <libavcodec/avcodec.h>
}

class Encoder {
  public:
    Encoder(std::ofstream &output_file, int fps, int width, int height);
    int encode(uint8_t *rgb);
    void finish();
    ~Encoder();

  private:
    void yuv_from_rgb(uint8_t *rgb);
    AVCodecContext *c;
    AVFrame *frame;
    struct SwsContext *sws_context = nullptr;
    std::ofstream &output_file;
};
