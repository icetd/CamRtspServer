#include <list>
#include <string.h>
#include "transcoder.h"
#include "INIReader.h"
#include "log.h"

TransCoder::TransCoder()
{
    init();
}

TransCoder::~TransCoder()
{
}

void TransCoder::init()
{
    std::list<uint32_t> formatList;
    v4l2IoType ioTypeIn  = IOTYPE_MMAP;
    INIReader configs("./configs/config.ini");
    if (configs.ParseError() < 0) {
        LOG(ERROR, "read video config failed.");
        return;
    } else {
        config.width = configs.GetInteger("video", "width", 640);
		config.height = configs.GetInteger("video", "height", 480);
		config.fps = configs.GetInteger("video", "fps", 30);
		config.method = configs.Get("video", "rc_method", "CRF");
		config.bitrate = configs.GetInteger("video", "bitrate", 1440);
		config.rf_constant = configs.GetInteger("video", "rf_constant", 23);
		config.format = configs.Get("video", "format", "UNKNOWN");
        config.device_name = configs.Get("video", "device", "/dev/video0");

		if (config.format == "YUY2") {
			formatList.push_back(V4L2_PIX_FMT_YUYV);
		} else if (config.format == "MJPEG") {
			formatList.push_back(V4L2_PIX_FMT_MJPEG);
		} 
    }

    V4L2DeviceParameters param(config.device_name.c_str(), 
                               formatList, 
                               config.width, 
                               config.height, 
                               config.fps, 
                               ioTypeIn, 
                               DEBUG);
    
    capture = V4l2Capture::create(param);
    
    X264_Param_t x264_param = {
        X264_CSP_I422,
        config.width,
        config.height,
        config.fps,
        config.method,
        config.bitrate,
        config.rf_constant
    };
    encoder = new X264Encoder(x264_param);

    decompress = new DeCompress();
    encodeData = (uint8_t*)malloc(capture->getBufferSize());
}

void TransCoder::run()
{
    timeval tv;
    for (;;) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int startCode = 0;
        int ret = capture->isReadable(&tv);
        if (ret == 1)
        {
            uint8_t buffer[capture->getBufferSize()];
            int resize = capture->read((char *)buffer, sizeof(buffer));
            frameSize = 0;
            if (config.format == "MJPEG") {
                ret = decompress->tjpeg2yuv(buffer, resize, &yuv_buf, &yuv_size);
                if (ret >= 0) {
                    frameSize = encoder->encode(yuv_buf, yuv_size, encodeData, config.format);
                    free(yuv_buf);               
                }
            } else if (config.format == "YUY2") {
                frameSize = encoder->encode(buffer, resize, encodeData, config.format);
            }
            LOG(INFO, "encodeData size %d", frameSize);
            
            if (encoder->startCode3((char *)encodeData)) 
                startCode = 3;
            else 
                startCode = 4;
			
            if (onEncodedDataCallback && frameSize > 0) {
                onEncodedDataCallback(std::vector<uint8_t>(encodeData + startCode, encodeData + frameSize));
            }

        } else if (ret == -1) {
            LOG(ERROR, "stop %s", strerror(errno));
        }
    }
}

TransCoder::Config_t const &TransCoder::getConfig() const
{
    return config;
}

void TransCoder::setOnEncoderDataCallback(std::function<void(std::vector<uint8_t> &&)> callback)
{
    onEncodedDataCallback = std::move(callback);
}
