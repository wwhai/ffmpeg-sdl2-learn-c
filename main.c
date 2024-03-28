#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include "clog.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
const char *out_filename = "rtmp://127.0.0.1:1935/live/testv001";

// 初始化流推送的函数
int initialize_streaming(AVFormatContext **inFmtCtx, AVFormatContext **outFmtCtx, const char *input_filename)
{
    // 打开输入文件
    if (avformat_open_input(inFmtCtx, input_filename, NULL, NULL) < 0)
    {
        log_error("Could not open input file.");
        return -1;
    }
    av_dump_format(*inFmtCtx, 0, "./1.mp4", 0);
    // 查找流信息
    if (avformat_find_stream_info(*inFmtCtx, NULL) < 0)
    {
        log_error("Could not find stream information.");
        avformat_close_input(inFmtCtx);
        return -1;
    }
    // 分配输出上下文
    if (avformat_alloc_output_context2(outFmtCtx, NULL, "flv", out_filename) < 0)
    {
        log_error("Could not create output context");
        avformat_close_input(inFmtCtx);
        return -1;
    }
    av_dump_format(*outFmtCtx, 0, out_filename, 0);
    // 为每个流创建输出流
    for (int i = 0; i < (*inFmtCtx)->nb_streams; i++)
    {
        AVStream *in_stream = (*inFmtCtx)->streams[i];
        AVStream *out_stream = avformat_new_stream(*outFmtCtx, NULL);
        if (!out_stream)
        {
            log_error("Failed allocating output stream");
            avformat_close_input(inFmtCtx);
            avformat_free_context(*outFmtCtx);
            return -1;
        }

        // 复制编解码器参数
        if (avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar) < 0)
        {
            log_error("Failed to copy codec parameters");
            avformat_close_input(inFmtCtx);
            avformat_free_context(*outFmtCtx);
            return -1;
        }
        out_stream->codecpar->codec_tag = 0;
    }

    // 打开输出URL
    if (!((*outFmtCtx)->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&(*outFmtCtx)->pb, out_filename, AVIO_FLAG_WRITE) < 0)
        {
            log_error("Could not open output URL '%s'", out_filename);
            avformat_close_input(inFmtCtx);
            avformat_free_context(*outFmtCtx);
            return -1;
        }
    }

    // 写入头部信息
    if (avformat_write_header(*outFmtCtx, NULL) < 0)
    {
        log_error("Error occurred when opening output URL");
        avformat_close_input(inFmtCtx);
        if (!((*outFmtCtx)->oformat->flags & AVFMT_NOFILE))
        {
            avio_closep(&(*outFmtCtx)->pb);
        }
        avformat_free_context(*outFmtCtx);
        return -1;
    }

    return 0;
}

int main()
{
    avformat_network_init();

    AVFormatContext *inFmtCtx = NULL;
    AVFormatContext *outFmtCtx = NULL;
    const char *input_filename = "./1.mp4";

    // 主循环
    while (1)
    {
        log_debug("initialize_streaming()");
        // 初始化流推送
        if (initialize_streaming(&inFmtCtx, &outFmtCtx, input_filename) < 0)
        {
            log_error("Failed to initialize streaming, exiting...");
            break;
        }

        // 读取并推送帧
        while (1)
        {
            AVPacket pkt;
            int ret = av_read_frame(inFmtCtx, &pkt);
            if (ret < 0)
            {
                if (ret == AVERROR_EOF)
                {
                    log_debug("av_read_frame -> AVERROR_EOF; reinitialize_streaming");
                    av_write_trailer(outFmtCtx);
                    avformat_close_input(&inFmtCtx);
                    if (outFmtCtx && !(outFmtCtx->oformat->flags & AVFMT_NOFILE))
                    {
                        avio_closep(&outFmtCtx->pb);
                    }
                    avformat_free_context(outFmtCtx);
                    break; // 跳过后面的清理代码，重新开始主循环
                }
                else
                {
                    log_error("Error reading frame: %d", ret);
                    break;
                }
            }
            // 推送帧到输出流
            ret = av_interleaved_write_frame(outFmtCtx, &pkt);
            // log_debug("pos=%ld, pts=%ld, ret=%d", pkt.pos, pkt.pts, ret);
            if (ret < 0)
            {
                log_error("Error muxing packet");
                break;
            }
            av_packet_unref(&pkt);
#ifdef _WIN32
            Sleep(1);
#else
            usleep(1);
#endif
        }
    }

    avformat_network_deinit();
    log_debug("avformat_network_deinit()");
    return 0;
}
