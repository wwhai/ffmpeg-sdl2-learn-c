#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <unistd.h>
void save_frame_as_bmp(AVFrame *frame, const char *filename)
{
    FILE *f;
    int width = 640;
    int height = 640;
    int row_size = (width * 3 + 3) & ~3; // Row size in bytes, aligned to 4 bytes
    int data_size = row_size * height;
    int file_size = 54 + data_size; // 54 bytes for BMP header and DIB header

    uint8_t bmp_header[54] = {
        'B', 'M',           // Signature
        0, 0, 0, 0,         // Image file size
        0, 0,               // Reserved1
        0, 0,               // Reserved2
        54, 0, 0, 0,        // Offset to start of image data
        40, 0, 0, 0,        // Header size
        width, 0, 0, 0,     // Image width
        height, 0, 0, 0,    // Image height
        1, 0,               // Number of color planes
        24, 0,              // Bits per pixel
        0, 0, 0, 0,         // Compression
        data_size, 0, 0, 0, // Image data size
        0x13, 0x0B, 0, 0,   // Horizontal resolution (2835 pixels/meter)
        0x13, 0x0B, 0, 0,   // Vertical resolution (2835 pixels/meter)
        0, 0, 0, 0,         // Number of colors
        0, 0, 0, 0          // Important colors
    };

    // Set file size and image offset
    bmp_header[2] = (file_size & 0xFF);
    bmp_header[3] = ((file_size >> 8) & 0xFF);
    bmp_header[4] = ((file_size >> 16) & 0xFF);
    bmp_header[5] = ((file_size >> 24) & 0xFF);

    bmp_header[10] = (54 & 0xFF);
    bmp_header[11] = ((54 >> 8) & 0xFF);
    bmp_header[12] = ((54 >> 16) & 0xFF);
    bmp_header[13] = ((54 >> 24) & 0xFF);

    bmp_header[18] = (width & 0xFF);
    bmp_header[19] = ((width >> 8) & 0xFF);
    bmp_header[20] = ((width >> 16) & 0xFF);
    bmp_header[21] = ((width >> 24) & 0xFF);

    bmp_header[22] = (height & 0xFF);
    bmp_header[23] = ((height >> 8) & 0xFF);
    bmp_header[24] = ((height >> 16) & 0xFF);
    bmp_header[25] = ((height >> 24) & 0xFF);

    bmp_header[34] = (data_size & 0xFF);
    bmp_header[35] = ((data_size >> 8) & 0xFF);
    bmp_header[36] = ((data_size >> 16) & 0xFF);
    bmp_header[37] = ((data_size >> 24) & 0xFF);

    f = fopen(filename, "wb");
    if (!f)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    fwrite(bmp_header, 1, 54, f);

    uint8_t *row = (uint8_t *)malloc(row_size);
    if (!row)
    {
        fprintf(stderr, "Memory allocation error\n");
        fclose(f);
        return;
    }

    for (int i = height - 1; i >= 0; i--)
    {
        memcpy(row, frame->data[0] + i * frame->linesize[0], width * 3);
        fwrite(row, 1, row_size, f);
    }

    free(row);
    fclose(f);
}

int main(int argc, char *argv[])
{
    const char *input_url = "rtsp://192.168.10.244:554/av0_0";
    const char *output_url = "rtmp://192.168.10.163:1935/live/test002";

    AVFormatContext *input_fmt_ctx = NULL, *output_fmt_ctx = NULL;
    AVCodecContext *input_codec_ctx = NULL, *output_codec_ctx = NULL;
    AVStream *video_stream = NULL;
    AVPacket packet;
    AVFrame *frame = NULL;
    AVFrame *rgb_frame = NULL;

    int video_stream_index = -1;
    int ret;

    // Open the input RTSP stream
    if (avformat_open_input(&input_fmt_ctx, input_url, NULL, NULL) < 0)
    {
        fprintf(stderr, "Could not open input stream\n");
        return 1;
    }

    if (avformat_find_stream_info(input_fmt_ctx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream info\n");
        return 1;
    }

    // Find the first video stream
    for (int i = 0; i < input_fmt_ctx->nb_streams; i++)
    {
        if (input_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index < 0)
    {
        fprintf(stderr, "No video stream found\n");
        return 1;
    }

    // Find the decoder for the video stream
    AVCodecParameters *codecpar = input_fmt_ctx->streams[video_stream_index]->codecpar;
    const AVCodec *input_codec = avcodec_find_decoder_by_name("h264");
    if (!input_codec)
    {
        fprintf(stderr, "Codec not found\n");
        return 1;
    }

    // Allocate a codec context for the decoder
    input_codec_ctx = avcodec_alloc_context3(input_codec);
    if (!input_codec_ctx)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        return 1;
    }

    // Copy codec parameters from input stream to codec context
    if (avcodec_parameters_to_context(input_codec_ctx, codecpar) < 0)
    {
        fprintf(stderr, "Could not copy codec parameters to context\n");
        return 1;
    }

    // Open the codec
    if (avcodec_open2(input_codec_ctx, input_codec, NULL) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        return 1;
    }

    // Create the output format context for RTMP
    if (avformat_alloc_output_context2(&output_fmt_ctx, NULL, "flv", output_url) < 0)
    {
        fprintf(stderr, "Could not create output context\n");
        return 1;
    }

    // Create a new video stream in the output context
    video_stream = avformat_new_stream(output_fmt_ctx, NULL);
    if (!video_stream)
    {
        fprintf(stderr, "Could not create new stream\n");
        return 1;
    }

    // Find and open the encoder for the output stream
    const AVCodec *output_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    // const AVCodec *output_codec = avcodec_find_encoder_by_name("h264");
    if (!output_codec)
    {
        fprintf(stderr, "Encoder not found\n");
        return 1;
    }

    output_codec_ctx = avcodec_alloc_context3(output_codec);
    if (!output_codec_ctx)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        return 1;
    }

    output_codec_ctx->height = input_codec_ctx->height;
    output_codec_ctx->width = input_codec_ctx->width;
    output_codec_ctx->sample_aspect_ratio = input_codec_ctx->sample_aspect_ratio;
    output_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    output_codec_ctx->time_base = (AVRational){1, 25}; // Assume 25 fps for simplicity

    if (output_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    {
        output_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(output_codec_ctx, output_codec, NULL) < 0)
    {
        fprintf(stderr, "Could not open encoder\n");
        return 1;
    }

    // Copy codec parameters from codec context to output stream
    if (avcodec_parameters_from_context(video_stream->codecpar, output_codec_ctx) < 0)
    {
        fprintf(stderr, "Could not copy codec parameters to output stream\n");
        return 1;
    }

    // Open the output file
    if (!(output_fmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&output_fmt_ctx->pb, output_url, AVIO_FLAG_WRITE) < 0)
        {
            fprintf(stderr, "Could not open output file\n");
            return 1;
        }
    }

    // Write the stream header
    if (avformat_write_header(output_fmt_ctx, NULL) < 0)
    {
        fprintf(stderr, "Error occurred while writing header\n");
        return 1;
    }

    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate frame\n");
        return 1;
    }
    rgb_frame = av_frame_alloc();
    if (!rgb_frame)
    {
        fprintf(stderr, "Could not allocate frame\n");
        return 1;
    }
    struct SwsContext *sws_ctx = sws_getContext(
        input_codec_ctx->width, input_codec_ctx->height, input_codec_ctx->pix_fmt,
        640, 640, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
    // int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, 640, 640, 1); //
    uint8_t *rgb_buffer = (uint8_t *)av_malloc(1228800);
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, rgb_buffer, AV_PIX_FMT_RGB24, 640, 640, 1);

    while (av_read_frame(input_fmt_ctx, &packet) >= 0)
    {
        if (packet.stream_index == video_stream_index)
        {
            if (avcodec_send_packet(input_codec_ctx, &packet) < 0)
            {
                fprintf(stderr, "Error sending packet to decoder\n");
                av_packet_unref(&packet);
                continue;
            }
            while (avcodec_receive_frame(input_codec_ctx, frame) >= 0)
            {
                //-------------------------------------------------------------
                // 格式转换成RGB，640*640
                //-------------------------------------------------------------
                sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0,
                          input_codec_ctx->height, rgb_frame->data, rgb_frame->linesize);
                // printf("write_bmp rgb_frame.bmp\n");
                // save_frame_as_bmp(rgb_frame, "./rgb_frame.bmp");
                frame->pts = frame->best_effort_timestamp;
                if (avcodec_send_frame(output_codec_ctx, frame) < 0)
                {
                    fprintf(stderr, "Error sending frame to encoder\n");
                    continue;
                }
                //
                // send_avframe_to_yolo8(0, frame);
                // receive_result_from_yolo8(0, NULL);
                //
                while (avcodec_receive_packet(output_codec_ctx, &packet) >= 0)
                {
                    AVStream *in_stream = input_fmt_ctx->streams[packet.stream_index];
                    AVStream *out_stream = output_fmt_ctx->streams[packet.stream_index];

                    packet.pts = av_rescale_q(packet.pts, in_stream->time_base, out_stream->time_base);
                    packet.dts = packet.pts;
                    packet.duration = packet.duration;
                    if (av_interleaved_write_frame(output_fmt_ctx, &packet) < 0)
                    {
                        fprintf(stderr, "Error writing packet to output file\n");
                    }
                    av_packet_unref(&packet);
                }
            }
        }
        av_packet_unref(&packet);
    }

    av_write_trailer(output_fmt_ctx);
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    avcodec_free_context(&input_codec_ctx);
    avcodec_free_context(&output_codec_ctx);
    avformat_close_input(&input_fmt_ctx);
    if (output_fmt_ctx && !(output_fmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&output_fmt_ctx->pb);
    }
    avformat_free_context(output_fmt_ctx);
    free(rgb_buffer);
    return 0;
}
