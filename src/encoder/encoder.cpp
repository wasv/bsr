#include "encoder.h"

#include <fstream>
#include <iostream>
#include <libavcodec/packet.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

Encoder::Encoder(std::ofstream &output_file, int fps, int width, int height) : output_file(output_file) {
    AVCodec *codec = avcodec_find_encoder_by_name("mpeg1video");
    if (!codec) {
        std::cerr << "Codec not found" << std::endl;
        exit(-3);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        std::cerr << "Could not allocate video codec context" << std::endl;
        exit(-3);
    }
    c->bit_rate = 0;
    c->width = width;
    c->height = height;
    c->time_base.num = 1;
    c->time_base.den = fps;
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    if (codec->id == AV_CODEC_ID_H264) {
        av_opt_set(c->priv_data, "preset", "slow", 0);
    }
    if (avcodec_open2(c, codec, NULL) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        exit(-3);
    }

    frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Could not allocate video frame" << std::endl;
        exit(-3);
    }
    frame->format = c->pix_fmt;
    frame->width = c->width;
    frame->height = c->height;
    if (av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32) < 0) {
        std::cerr << "Could not allocate raw picture buffer" << std::endl;
        exit(-3);
    }
}

void Encoder::yuv_from_rgb(uint8_t *rgb) {
    const int in_linesize[1] = {4 * c->width};
    sws_context = sws_getCachedContext(sws_context, c->width, c->height, AV_PIX_FMT_RGBA, c->width, c->height,
                                       AV_PIX_FMT_YUV420P, 0, 0, 0, 0);
    sws_scale(sws_context, (const uint8_t *const *)&rgb, in_linesize, 0, c->height, frame->data, frame->linesize);
}

int Encoder::encode(uint8_t *rgb) {
    int got_output;
    this->yuv_from_rgb(rgb);
    AVPacket *pkt = av_packet_alloc();
    frame->pts += 1;
    if (avcodec_send_frame(c, frame) < 0) {
        std::cerr << "Error sending frame to encoder." << std::endl;
        return -1;
    }

    int ret = avcodec_receive_packet(c, pkt);
    while (ret >= 0) {
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            std::cerr << "Error during encoding." << std::endl;
            av_packet_unref(pkt);
            return -2;
        }
        output_file.write((char *)pkt->data, pkt->size);

        ret = avcodec_receive_packet(c, pkt);
    }
    av_packet_free(&pkt);
    return 0;
}

void Encoder::finish() {
    uint8_t endcode[] = {0, 0, 1, 0xb7};
    output_file.write((char *)endcode, sizeof(endcode));
    output_file.close();
}

Encoder::~Encoder() {
    sws_freeContext(sws_context);
    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
}
