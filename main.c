#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "clog.h"
int main()
{
    const char *input_url = "rtsp://192.168.1.210:554/av0_0";
    const char *output_url = "rtmp://127.0.0.1:1935/live/testv001";

    avformat_network_init();

    AVFormatContext *input_ctx = NULL;
    AVFormatContext *output_ctx = NULL;
    AVPacket packet;
    int ret;

    // Open input RTSP stream
    ret = avformat_open_input(&input_ctx, input_url, NULL, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to open input RTSP stream: %s\n", av_err2str(ret));
        return ret;
    }

    // Retrieve stream information
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

    // Read packets from input and write to output
    while (1)
    {
        ret = av_read_frame(input_ctx, &packet);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF)
                break;
            fprintf(stderr, "Error reading packet: %s\n", av_err2str(ret));
            break;
        }

        AVStream *in_stream = input_ctx->streams[packet.stream_index];
        AVStream *out_stream = output_ctx->streams[packet.stream_index];

        packet.pts = av_rescale_q(packet.pts, in_stream->time_base, out_stream->time_base);
        packet.dts = packet.pts;
        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
        ret = av_interleaved_write_frame(output_ctx, &packet);
        if (ret < 0)
        {
            fprintf(stderr, "Error writing packet: %s\n", av_err2str(ret));
            av_packet_unref(&packet);
            break;
        }

        av_packet_unref(&packet);
#ifdef _WIN32
        Sleep(40);
#else
        usleep(40000);
#endif
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
