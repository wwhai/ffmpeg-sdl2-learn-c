#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "clog.h"
//
// 添加文本水印
void add_text_watermark(AVFrame *frame, const char *text)
{
    // 水印的位置和属性
    int x = 10;                              // 水印的 x 坐标
    int y = frame->height - 30;              // 水印的 y 坐标
    int font_size = 20;                      // 字体大小
    int font_thickness = 2;                  // 字体厚度
    uint8_t font_color[3] = {255, 255, 255}; // 字体颜色（白色）

    // 计算水印的宽度和高度
    int text_width = strlen(text) * font_size / 2;
    int text_height = font_size + font_thickness;

    // 确保水印在图像范围内
    if (x + text_width > frame->width || y - text_height < 0)
    {
        fprintf(stderr, "Watermark position is out of image bounds\n");
        return;
    }

    // 在图像上绘制水印
    for (int i = 0; i < strlen(text); i++)
    {
        for (int fy = 0; fy < font_size; fy++)
        {
            for (int fx = 0; fx < font_size / 2; fx++)
            {
                int pixel_x = x + i * font_size / 2 + fx;
                int pixel_y = y - fy;
                if (pixel_x >= 0 && pixel_x < frame->width && pixel_y >= 0 && pixel_y < frame->height)
                {
                    int pixel_pos = (pixel_y * frame->linesize[0]) + (pixel_x * 3);
                    memcpy(frame->data[0] + pixel_pos, font_color, 3);
                }
            }
        }
    }
}
//
const char *input_url = "rtsp://192.168.1.210:554/av0_0";
const char *output_url = "rtmp://127.0.0.1:1935/live/testv001";
int main()
{
    int ret;
    avformat_network_init();

    AVFormatContext *input_ctx = NULL;
    AVFormatContext *output_ctx = NULL;

    // Open input RTSP stream
    ret = avformat_open_input(&input_ctx, input_url, NULL, NULL);
    if (ret < 0)
    {
        log_error("Failed to open input RTSP stream: %s", av_err2str(ret));
        return ret;
    }

    ret = avformat_find_stream_info(input_ctx, NULL);
    if (ret < 0)
    {
        log_error("Failed to retrieve input stream information: %s", av_err2str(ret));
        avformat_close_input(&input_ctx);
        return ret;
    }

    // Open output RTMP stream
    ret = avformat_alloc_output_context2(&output_ctx, NULL, "flv", output_url);
    if (ret < 0)
    {
        log_error("Failed to create output context: %s", av_err2str(ret));
        avformat_close_input(&input_ctx);
        return ret;
    }
    // Add video stream to output context
    for (int i = 0; i < input_ctx->nb_streams; i++)
    {
        AVStream *in_stream = input_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(output_ctx, NULL);
        if (!out_stream)
        {
            log_error("Failed to create output stream");
            avformat_close_input(&input_ctx);
            avformat_free_context(output_ctx);
            return -1;
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0)
        {
            log_error("Failed to copy codec parameters: %s", av_err2str(ret));
            avformat_close_input(&input_ctx);
            avformat_free_context(output_ctx);
            return ret;
        }
    }
    // Open output URL for writing
    if (!(output_ctx->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&output_ctx->pb, output_url, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            log_error("Failed to open output URL: %s", av_err2str(ret));
            avformat_close_input(&input_ctx);
            avformat_free_context(output_ctx);
            return ret;
        }
    }
    // Write output header
    ret = avformat_write_header(output_ctx, NULL);
    if (ret < 0)
    {
        log_error("Failed to write header to output stream: %s", av_err2str(ret));
        avformat_close_input(&input_ctx);
        avformat_free_context(output_ctx);
        return ret;
    }
    //
    const AVCodec *codec = NULL;
    codec = avcodec_find_decoder(AV_CODEC_ID_H264); // 以H.264解码器为例
    if (!codec)
    {
        log_error("Codec not found");
        return -1;
    }
    AVCodecContext *codec_ctx = NULL;
    // 2. 分配解码器上下文
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        log_error("Could not allocate codec context");
        return -1;
    }
    // 3. 打开解码器
    // 现在，codec_ctx 就是解码器的上下文，可以用于解码操作
    ret = avcodec_open2(codec_ctx, codec, NULL);
    if (ret < 0)
    {
        log_error("Could not open codec: %s", av_err2str(ret));
        avcodec_free_context(&codec_ctx);
        return ret;
    }

    AVPacket *OriginPacket = av_packet_alloc();
    AVFrame *InFrame = av_frame_alloc();
    AVPacket *WatermarkPkt = av_packet_alloc();
    while (1)
    {
        ret = av_read_frame(input_ctx, OriginPacket);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF)
                break;
            log_error("Error reading packet: %s", av_err2str(ret));
            break;
        }
        // log_debug("av_read_frame %d\n", codec_ctx->frame_num);

        //------------------------------------------------------------------------------------------
        // 从协议里面解帧
        //------------------------------------------------------------------------------------------
        // if (OriginPacket->stream_index == 0)
        // {
        //     if (avcodec_send_packet(codec_ctx, OriginPacket) < 0)
        //     {
        //         log_error("avcodec_send_packet: %s", av_err2str(ret));
        //         continue;
        //     }
        //     if (avcodec_receive_frame(codec_ctx, InFrame) < 0)
        //     {
        //         log_debug("avcodec_receive_frame: %s", av_err2str(ret));
        //         continue;
        //     }
        //     // add_text_watermark(InFrame, "HELLOWORLD");
        //     if (avcodec_send_frame(codec_ctx, InFrame) < 0)
        //     {
        //         log_debug("avcodec_send_frame: %s", av_err2str(ret));
        //         continue;
        //     }
        //     if (avcodec_receive_packet(codec_ctx, WatermarkPkt) < 0)
        //     {
        //         log_debug("avcodec_receive_packet: %s", av_err2str(ret));
        //         continue;
        //     }
        // }
        // AVStream *in_stream = input_ctx->streams[OriginPacket->stream_index];
        // AVStream *out_stream = output_ctx->streams[OriginPacket->stream_index];

        // WatermarkPkt->pts = av_rescale_q(OriginPacket->pts, in_stream->time_base, out_stream->time_base);
        // WatermarkPkt->duration = av_rescale_q(OriginPacket->duration, in_stream->time_base, out_stream->time_base);
        // WatermarkPkt->dts = OriginPacket->pts;

        // ret = av_interleaved_write_frame(output_ctx, WatermarkPkt);
        // log_debug("av_interleaved_write_frame: %d", WatermarkPkt->dts);
        // if (ret < 0)
        // {
        //     log_error("Error writing packet: %d", (ret));
        //     break;
        // }
        //------------------------------------------------------------------------------------------
        // 转发
        //------------------------------------------------------------------------------------------
        AVStream *in_stream = input_ctx->streams[OriginPacket->stream_index];
        AVStream *out_stream = output_ctx->streams[OriginPacket->stream_index];

        OriginPacket->pts = av_rescale_q(OriginPacket->pts, in_stream->time_base, out_stream->time_base);
        OriginPacket->duration = av_rescale_q(OriginPacket->duration, in_stream->time_base, out_stream->time_base);
        OriginPacket->dts = OriginPacket->pts;

        ret = av_interleaved_write_frame(output_ctx, OriginPacket);
        // printf("av_interleaved_write_frame: %d", OriginPacket.dts);
        if (ret < 0)
        {
            log_error("Error writing packet: %s", av_err2str(ret));
            break;
        }
    }
    av_packet_free(&OriginPacket);
    av_packet_free(&WatermarkPkt);
    av_frame_free(&InFrame);
    ret = av_write_trailer(output_ctx);
    if (ret < 0)
    {
        log_error("Failed to write trailer to output stream: %s", av_err2str(ret));
        avformat_close_input(&input_ctx);
        avformat_free_context(output_ctx);
        return ret;
    }

    // Cleanup
    if (output_ctx && !(output_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&output_ctx->pb);
    }
    avformat_close_input(&input_ctx);
    avformat_free_context(output_ctx);
    avformat_network_deinit();
    return 0;
}
