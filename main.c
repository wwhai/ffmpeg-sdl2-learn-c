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

int AddWaterMark(AVFrame *frame_in, AVFrame *frame_out, int w, int h, const char *str)
{
    AVFilterGraph *filter_graph;
    AVFilterContext *buffersrc_ctx = NULL, *buffersink_ctx = NULL;
    const AVFilter *buffersrc, *buffersink;
    int ret;

    // Initialize filter graph
    filter_graph = avfilter_graph_alloc();
    if (!filter_graph)
    {
        fprintf(stderr, "Failed to allocate filter graph\n");
        return AVERROR(ENOMEM);
    }

    // Get buffersrc and buffersink filters
    buffersrc = avfilter_get_by_name("buffer");
    buffersink = avfilter_get_by_name("buffersink");
    if (!buffersrc || !buffersink)
    {
        fprintf(stderr, "Failed to get buffer source/sink filter\n");
        avfilter_graph_free(&filter_graph);
        return AVERROR_UNKNOWN;
    }

    // Create input filter chain
    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", NULL, NULL, filter_graph);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to create buffer source filter\n");
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // Create output filter chain
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, filter_graph);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to create buffer sink filter\n");
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // Initialize drawtext filter parameters
    char args[256];
    snprintf(args, sizeof(args),
             "text='%s':x=(w-text_w)/2:y=h-100:fontsize=24:fontcolor=white",
             str);

    // Create drawtext filter
    AVFilterContext *drawtext_ctx = NULL;
    const AVFilter *drawtext = avfilter_get_by_name("drawtext");
    if (!drawtext)
    {
        fprintf(stderr, "Failed to get drawtext filter\n");
        avfilter_graph_free(&filter_graph);
        return AVERROR_UNKNOWN;
    }

    // Create drawtext filter chain
    ret = avfilter_graph_create_filter(&drawtext_ctx, drawtext, "drawtext", args, NULL, filter_graph);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to create drawtext filter\n");
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // Link filters
    ret = avfilter_link(buffersrc_ctx, 0, drawtext_ctx, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to link buffersrc and drawtext filters\n");
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    ret = avfilter_link(drawtext_ctx, 0, buffersink_ctx, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to link drawtext and buffersink filters\n");
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // Configure the graph
    ret = avfilter_graph_config(filter_graph, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to configure filter graph\n");
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // Send frame to the filter graph
    ret = av_buffersrc_add_frame_flags(buffersrc_ctx, frame_in, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to send frame to filter graph\n");
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // Receive filtered frame from the filter graph
    AVFrame *filtered_frame = av_frame_alloc();
    if (!filtered_frame)
    {
        fprintf(stderr, "Failed to allocate filtered frame\n");
        avfilter_graph_free(&filter_graph);
        return AVERROR(ENOMEM);
    }

    ret = av_buffersink_get_frame(buffersink_ctx, filtered_frame);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to get filtered frame from filter graph\n");
        av_frame_free(&filtered_frame);
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // Convert filtered frame to the output frame format
    struct SwsContext *sws_ctx = sws_getContext(filtered_frame->width, filtered_frame->height, filtered_frame->format,
                                                w, h, AV_PIX_FMT_YUV420P,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx)
    {
        fprintf(stderr, "Failed to initialize scaling context\n");
        av_frame_free(&filtered_frame);
        avfilter_graph_free(&filter_graph);
        return AVERROR_UNKNOWN;
    }

    sws_scale(sws_ctx, (const uint8_t *const *)filtered_frame->data, filtered_frame->linesize,
              0, filtered_frame->height, frame_out->data, frame_out->linesize);
    frame_out->width = w;
    frame_out->height = h;

    // Free resources
    sws_freeContext(sws_ctx);
    av_frame_free(&filtered_frame);
    avfilter_graph_free(&filter_graph);

    return 0;
}
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
        fprintf(stderr, "Failed to open input RTSP stream: %s\n", av_err2str(ret));
        return ret;
    }

    ret = avformat_find_stream_info(input_ctx, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to retrieve input stream information: %s\n", av_err2str(ret));
        avformat_close_input(&input_ctx);
        return ret;
    }

    // Open output RTMP stream
    ret = avformat_alloc_output_context2(&output_ctx, NULL, "flv", output_url);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to create output context: %s\n", av_err2str(ret));
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
            fprintf(stderr, "Failed to create output stream\n");
            avformat_close_input(&input_ctx);
            avformat_free_context(output_ctx);
            return -1;
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0)
        {
            fprintf(stderr, "Failed to copy codec parameters: %s\n", av_err2str(ret));
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
            fprintf(stderr, "Failed to open output URL: %s\n", av_err2str(ret));
            avformat_close_input(&input_ctx);
            avformat_free_context(output_ctx);
            return ret;
        }
    }
    // Write output header
    ret = avformat_write_header(output_ctx, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to write header to output stream: %s\n", av_err2str(ret));
        avformat_close_input(&input_ctx);
        avformat_free_context(output_ctx);
        return ret;
    }
    //
    const AVCodec *codec = NULL;
    codec = avcodec_find_decoder(AV_CODEC_ID_H264); // 以H.264解码器为例
    if (!codec)
    {
        fprintf(stderr, "Codec not found\n");
        return -1;
    }
    AVCodecContext *codec_ctx = NULL;
    // 2. 分配解码器上下文
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        fprintf(stderr, "Could not allocate codec context\n");
        return -1;
    }
    // 3. 打开解码器
    // 现在，codec_ctx 就是解码器的上下文，可以用于解码操作
    ret = avcodec_open2(codec_ctx, codec, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
        avcodec_free_context(&codec_ctx);
        return ret;
    }
    //---
    AVPacket OriginPacket;
    AVFrame DecodedFrame;
    //---
    while (1)
    {
        printf("1(0)\n");
        // 接收解码后的帧
        ret = avcodec_receive_frame(codec_ctx, &DecodedFrame);
        if (ret < 0)
        {
            fprintf(stderr, "Error receiving frame from decoder: %s\n", av_err2str(ret));
            av_packet_unref(&OriginPacket);
            av_frame_unref(&DecodedFrame);
            return ret;
        }
        printf("2(0)\n");

        AVFrame FilteredFrame;
        AVPacket NewPacket;
        printf("AddWaterMark(0)\n");
        AddWaterMark(&DecodedFrame, &FilteredFrame, 0, 0, "HELLO WORLD");
        avcodec_send_frame(codec_ctx, &FilteredFrame);
        while (avcodec_receive_packet(codec_ctx, &NewPacket) == 0)
        {
            av_packet_unref(&NewPacket);
        }
        av_frame_unref(&FilteredFrame);
        //
        //
        //
        ret = av_read_frame(input_ctx, &OriginPacket);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF)
                break;
            fprintf(stderr, "Error reading packet: %s\\n", av_err2str(ret));
            break;
        }
        AVStream *in_stream = input_ctx->streams[OriginPacket.stream_index];
        AVStream *out_stream = output_ctx->streams[OriginPacket.stream_index];

        OriginPacket.pts = av_rescale_q(OriginPacket.pts, in_stream->time_base, out_stream->time_base);
        OriginPacket.duration = av_rescale_q(OriginPacket.duration, in_stream->time_base, out_stream->time_base);
        OriginPacket.dts = OriginPacket.pts;

        ret = av_interleaved_write_frame(output_ctx, &OriginPacket);
        if (ret < 0)
        {
            fprintf(stderr, "Error writing packet: %s\n", av_err2str(ret));
            goto final;
            break;
        }
    final:
        av_packet_unref(&OriginPacket);
    }
    ret = av_write_trailer(output_ctx);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to write trailer to output stream: %s\n", av_err2str(ret));
        avformat_close_input(&input_ctx);
        avformat_free_context(output_ctx);
        return ret;
    }

    // Cleanup
    avformat_close_input(&input_ctx);
    if (output_ctx && !(output_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&output_ctx->pb);
    avformat_free_context(output_ctx);

    avformat_network_deinit();
    return 0;
}
