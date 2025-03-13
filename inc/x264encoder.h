#ifndef X264_ENCODER_H
#define X264_ENCODER_H

#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <x264.h>
#include <stdio.h>
#include <string>

typedef struct {
    int colorSpace;
    int width;
    int height;
    int fps;
    std::string method;
    int bitrate;
    int rf_constant;
    int min_ikeyint;
    int max_ikeyint;
} X264_Param_t;

class X264Encoder
{
public:
    X264Encoder(const X264_Param_t param);
    X264Encoder(X264_Param_t, int threadId);
    virtual ~X264Encoder();

    int initialize();
    int encode(uint8_t *inbuf, int insize, uint8_t *outbuf, std::string &format);

    int startCode3(char *buf);
    int startCode4(char *buf);

private:
    X264_Param_t m_x264_param;
    int m_threadId;

    x264_t *pHandle = nullptr;
    x264_param_t *param = nullptr;
    x264_picture_t *pic_in = nullptr;
    x264_picture_t *pic_out = nullptr;

    x264_nal_t *nals = nullptr;
    int num_nals = 0;
    int pts = 0;
    bool isStop = false;
};

#endif
