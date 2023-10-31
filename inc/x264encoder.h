#ifndef X264_ENCODER_H
#define X264_ENCODER_H

#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <x264.h>
#include <stdio.h>
#include <string>



class X264Encoder {
public:
	X264Encoder(const int width, const int height, const int colorSpace);
	X264Encoder(const int width, const int height, const int colorSpace, int threadId);
	virtual ~X264Encoder();

	int initialize();
	int encode(uint8_t *inbuf, int insize, uint8_t *outbuf, std::string &format);
	
	int startCode3(char *buf);
	int startCode4(char *buf);
private:
	int m_width;
	int m_height;
	int m_colorSpace;
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
