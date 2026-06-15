#include <bits/stdint-uintn.h>
#include <x264encoder.h>
#include <log.h>
#include <cstring>
#include <stdlib.h>

X264Encoder::X264Encoder(X264_Param_t param) :
    m_x264_param(param),
    m_threadId(0)
{
    initialize();
}

X264Encoder::~X264Encoder()
{
    free(param);
    free(pic_in);
    free(pic_out);
}

int X264Encoder::initialize()
{
    isStop = false;

    param = (x264_param_t *)malloc(sizeof(x264_param_t));
    pic_in = (x264_picture_t *)malloc(sizeof(x264_picture_t));
    pic_out = (x264_picture_t *)malloc(sizeof(x264_picture_t));

    pic_in->i_pts = 0;
    x264_param_default(param);

    param->i_csp = m_x264_param.colorSpace;
    param->i_threads = m_threadId;
    param->i_width = m_x264_param.width;
    param->i_height = m_x264_param.height;
    param->i_fps_den = 1;
    param->i_fps_num = m_x264_param.fps;

    if (m_x264_param.method == "CRF") {
        param->rc.f_rf_constant = m_x264_param.rf_constant;
        param->rc.i_rc_method = X264_RC_CRF;
    } else if (m_x264_param.method == "ABR") {
        param->rc.i_bitrate = m_x264_param.bitrate;
        param->rc.i_rc_method = X264_RC_ABR;
    }

    param->i_keyint_min = m_x264_param.min_ikeyint;
    param->i_keyint_max = m_x264_param.max_ikeyint;

    /* Use minimum latency ultrafast*/
    param->i_sync_lookahead = 0;
    param->b_sliced_threads = 4;
    param->b_vfr_input = 0;

    param->b_cabac = 0;
    /* 没有B帧
     */
    param->i_bframe = 0;
    /* 设置intra划分的partition，这里应该是不进行划分，只使用16x16
     */
    param->analyse.intra = 0;
    /* 设置inter划分的partition，这里应该是不进行划分，只使用16x16
     */
    param->analyse.inter = 0;
    /* 不使用8x8的的DCT变换
     */
    param->analyse.b_transform_8x8 = 0;
    /* 运动估计：DIA
     */
    param->analyse.i_me_method = X264_ME_DIA;
    /* 亚像素运动估计精度：0,表示只进行整像素运动估计
     */
    param->analyse.i_subpel_refine = 0;
    /* 关闭自适应量化
     */
    param->rc.i_aq_mode = 0;
    /* 禁止每个宏块的分区拥有自己的参考编号
     */
    param->analyse.b_mixed_references = 0;
    /* 不使用trellis优化
     */
    param->analyse.i_trellis = 0;
    /* 自适应B帧判断的概率（从-100到100）
     */
    param->i_bframe_adaptive = X264_B_ADAPT_NONE;
    /* 不使用mbtree
     */
    param->rc.b_mb_tree = 0;
    /* 权重预测中，P帧的权值
     */
    param->analyse.i_weighted_pred = X264_WEIGHTP_NONE;
    /* 显式的B帧权重预测
     */
    param->analyse.b_weighted_bipred = 0;
    /* mbtree前向预测的帧的数量
     */
    param->rc.i_lookahead = 0;

    x264_param_apply_profile(param, "high422");

    pHandle = x264_encoder_open(param);

    x264_picture_init(pic_out);
    x264_picture_alloc(pic_in, param->i_csp, param->i_width, param->i_height);

    pic_in->img.i_csp = param->i_csp;
    pic_in->img.i_plane = 3;

    if (!pHandle) {
        LOG(ERROR, "open x264 encoder failed.");
        return -1;
    }

    return 0;
}
/**  @brief encode YUV422 to H.264
 *   @param 	inbuf  YUV422 frame
 *   @param 	insize YUV422 frame size
 *   @param 	outbuf H.264 frame
 *   @param     format YUV422 fotmat |YUY2 camera| or |YUV422 from DeCompress|
 *   @return    the size of H.264 frame
 */
int X264Encoder::encode(uint8_t *inbuf, int insize, uint8_t *outbuf, std::string &format)
{
    int size = 0;
    uint8_t *out = outbuf;
    
    int width = m_x264_param.width;
    int height = m_x264_param.height;
    
    if (format == "MJPEG") {
        // MJPEG 解码后应该是 YUV420P 或者 YUV422P
        int y_size = width * height;
        int uv_size = y_size / 4;
        uint8_t *y = pic_in->img.plane[0];
        uint8_t *u = pic_in->img.plane[1];
        uint8_t *v = pic_in->img.plane[2];

        if (insize == y_size * 3 / 2) {
            // YUV420P
            memcpy(y, inbuf, y_size);
            memcpy(u, inbuf + y_size, y_size / 4);
            memcpy(v, inbuf + y_size + y_size / 4, y_size / 4);
        } else if (insize == y_size * 2) {
            // YUV422P
            memcpy(y, inbuf, y_size);

            uint8_t *src_u = inbuf + y_size;
            uint8_t *src_v = src_u + y_size / 2;

            // 422 -> 420
            for (int row = 0; row < height / 2; row++) {
                memcpy(
                    u + row * width / 2,
                    src_u + (row * 2) * width / 2,
                    width / 2);

                memcpy(
                    v + row * width / 2,
                    src_v + (row * 2) * width / 2,
                    width / 2);
            }
        }
    } 
    else if (format == "YUY2") {
        // YUY2 格式：Y0 U0 Y1 V0 Y2 U1 Y3 V1 ...
        // 需要转换为 YUV420P（Y 平面大小 w*h，U/V 平面大小 w/2 * h/2）
        
        uint8_t *y = pic_in->img.plane[0];
        uint8_t *u = pic_in->img.plane[1];
        uint8_t *v = pic_in->img.plane[2];
        
        int y_index = 0;
        int u_index = 0;
        int v_index = 0;

        for (int h = 0; h < height; h += 2) {
            for (int w = 0; w < width; w += 2) {
                int p0 = (h * width + w) * 2;
                int p1 = (h * width + w + 1) * 2;
                int p2 = ((h + 1) * width + w) * 2;
                int p3 = ((h + 1) * width + w + 1) * 2;

                uint8_t Y0 = inbuf[p0];
                uint8_t U = inbuf[p0 + 1];
                uint8_t Y1 = inbuf[p1];
                uint8_t V = inbuf[p1 + 1];

                y[(h * width) + w] = Y0;
                y[(h * width) + w + 1] = Y1;
                y[((h + 1) * width) + w] = inbuf[p2];
                y[((h + 1) * width) + w + 1] = inbuf[p3];

                u[(h / 2) * (width / 2) + (w / 2)] = U;
                v[(h / 2) * (width / 2) + (w / 2)] = V;
            }
        }
    }

    pic_in->i_pts = pts;
    pts++;

    int ret = x264_encoder_encode(pHandle, &nals, &num_nals, pic_in, pic_out);
    if (ret < 0) {
        LOG(ERROR, "x264 encode failed.");
        return -1;
    }

    for (int j = 0; j < num_nals; j++) {
        memcpy(out, nals[j].p_payload, nals[j].i_payload);
        size += nals[j].i_payload;
        out += nals[j].i_payload;
    }
    
    if (size > 0 && (pts % 30 == 0)) {
        LOG(INFO, "h264 pts:%d frame size: %d", pic_in->i_pts, size);
    }

    return size;
}

int X264Encoder::startCode3(char *buf)
{
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return 1;
    else
        return 0;
}

int X264Encoder::startCode4(char *buf)
{
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return 1;
    else
        return 0;
}
