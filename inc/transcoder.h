#ifndef TRANS_CODE_H
#define TRANS_CODE_H

#include <functional>
#include <atomic>
#include <string>
#include <vector>

#include "MThread.h"
#include "DeCompress.h"
#include "x264encoder.h"
#include "V4l2Device.h"
#include "V4l2Capture.h"


class TransCoder : public MThread
{
public:
    typedef struct {
        int width;
        int height;
        int fps;
        std::string method;
        int bitrate;
        int rf_constant;
        int min_ikeyint;
        int max_ikeyint;
        std::string format;
        std::string device_name;
    } Config_t;

    TransCoder();
    ~TransCoder();
    void init();

    void run() override;
    void setOnEncoderDataCallback(std::function<void(std::vector<uint8_t> &&)> callback);

    TransCoder::Config_t const &getConfig() const;
private:
    Config_t config;
    
    X264Encoder *encoder;
    V4l2Capture *capture;
    DeCompress *decompress;
    uint8_t *yuv_buf;
    int yuv_size;
    uint8_t *encodeData;
    int frameSize;

    std::function<void(std::vector<uint8_t> &&)> onEncodedDataCallback;
};


#endif