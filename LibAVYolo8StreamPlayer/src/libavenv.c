/**
 * Copyright (C) 2024 wwhai
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#define LIBAVENV
#ifdef LIBAVENV

#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <stdlib.h>
typedef struct TLibAVEnv
{
    AVFormatContext *inputFmtCtx;
    AVFormatContext *outputFmtCtx;
    int videoInstreamIndex;
    int audioInstreamIndex;
    AVCodecContext *inputAudioCodecCtx;
    AVCodecContext *inputVideoCodecCtx;
    const AVCodec *inputAudioCodec;
    const AVCodec *inputVideoCodec;
    AVCodecContext *outputCodecCtx;
    const AVCodec *outputCodec;
    AVStream *outputVideoStream;
    AVPacket *OnePacket;
    AVFrame *OneFrame;
    AVFrame *yoloFrame;
    struct SwsContext *swsCtx;

} TLibAVEnv;
TLibAVEnv *NewTLibAVEnv()
{
    av_log_set_level(AV_LOG_ERROR);
    TLibAVEnv *Env = (TLibAVEnv *)malloc(sizeof(TLibAVEnv));
    Env->videoInstreamIndex = -1;
    Env->audioInstreamIndex = -1;
    Env->inputFmtCtx = NULL;
    Env->outputFmtCtx = NULL;
    Env->inputAudioCodecCtx = NULL;
    Env->inputVideoCodecCtx = NULL;
    Env->outputCodecCtx = NULL;
    Env->outputVideoStream = NULL;
    Env->OnePacket = NULL;
    Env->OneFrame = NULL;
    Env->yoloFrame = NULL;
    Env->outputCodec = NULL;
    Env->inputVideoCodec = NULL;
    Env->inputAudioCodec = NULL;
}
int OpenStream(TLibAVEnv *Env, const char *inputUrl, const char *outputUrl)
{
    int ret;
    char error_buffer[128];
    if (ret = avformat_open_input(&Env->inputFmtCtx, inputUrl, NULL, NULL) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avformat_open_input: %s\n", error_buffer);
        return 1;
    }

    if (avformat_find_stream_info(Env->inputFmtCtx, NULL) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avformat_find_stream_info: %s\n", error_buffer);
        return 1;
    }
    for (int i = 0; i < Env->inputFmtCtx->nb_streams; i++)
    {
        if (Env->inputFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            Env->videoInstreamIndex = i;
            break;
        }
    }
    if (Env->videoInstreamIndex < 0)
    {
        fprintf(stderr, "!= videoInstreamIndex\n");
        return 1;
    }
    for (int i = 0; i < Env->inputFmtCtx->nb_streams; i++)
    {
        if (Env->inputFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            Env->audioInstreamIndex = i;
            break;
        }
    }
    if (Env->audioInstreamIndex < 0)
    {
        fprintf(stderr, "!= audioInstreamIndex\n");
        return 1;
    }
    AVCodecParameters *audioCodecpar = Env->inputFmtCtx->streams[Env->audioInstreamIndex]->codecpar;
    Env->inputAudioCodec = avcodec_find_decoder(audioCodecpar->codec_id);
    if (!Env->inputAudioCodec)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_find_decoder audioCodecpar: %s\n", error_buffer);
        return 1;
    }
    AVCodecParameters *videoCodecpar = Env->inputFmtCtx->streams[Env->videoInstreamIndex]->codecpar;
    Env->inputVideoCodec = avcodec_find_decoder(videoCodecpar->codec_id);
    if (!Env->inputVideoCodec)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_find_decoder videoCodecpar: %s\n", error_buffer);
        return 1;
    }
    Env->inputAudioCodecCtx = avcodec_alloc_context3(Env->inputAudioCodec);
    if (!Env->inputAudioCodecCtx)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_alloc_context3 inputAudioCodecCtx: %s\n", error_buffer);
        return 1;
    }
    Env->inputVideoCodecCtx = avcodec_alloc_context3(Env->inputVideoCodec);
    if (!Env->inputVideoCodecCtx)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_alloc_context3 inputVideoCodecCtx: %s\n", error_buffer);
        return 1;
    }
    if (avcodec_parameters_to_context(Env->inputAudioCodecCtx, audioCodecpar) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_parameters_to_context inputAudioCodecCtx: %s\n", error_buffer);
        return 1;
    }
    if (avcodec_parameters_to_context(Env->inputVideoCodecCtx, videoCodecpar) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_parameters_to_context inputVideoCodecCtx: %s\n", error_buffer);
        return 1;
    }
    if (avcodec_open2(Env->inputAudioCodecCtx, Env->inputAudioCodec, NULL) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_open2 inputAudioCodecCtx: %s\n", error_buffer);
        return 1;
    }
    if (avcodec_open2(Env->inputVideoCodecCtx, Env->inputVideoCodec, NULL) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_open2 inputVideoCodecCtx: %s\n", error_buffer);
        return 1;
    }

    if (avformat_alloc_output_context2(&Env->outputFmtCtx, NULL, "flv", outputUrl) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avformat_alloc_output_context2 outputFmtCtx: %s\n", error_buffer);
        return 1;
    }
    Env->outputVideoStream = avformat_new_stream(Env->outputFmtCtx, NULL);
    if (!Env->outputVideoStream)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avformat_new_stream outputFmtCtx: %s\n", error_buffer);
        return 1;
    }
    Env->outputCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!Env->outputCodec)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_find_encoder outputCodec AV_CODEC_ID_H264: %s\n", error_buffer);
        return 1;
    }

    Env->outputCodecCtx = avcodec_alloc_context3(Env->outputCodec);
    if (!Env->outputCodecCtx)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_alloc_context3 outputCodecCtx: %s\n", error_buffer);
        return 1;
    }

    Env->outputCodecCtx->height = Env->inputVideoCodecCtx->height;
    Env->outputCodecCtx->width = Env->inputVideoCodecCtx->width;
    Env->outputCodecCtx->sample_aspect_ratio = Env->inputVideoCodecCtx->sample_aspect_ratio;
    Env->outputCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    Env->outputCodecCtx->time_base = (AVRational){1, 25};

    if (Env->outputFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
    {
        Env->outputCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(Env->outputCodecCtx, Env->outputCodec, NULL) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_open2 outputCodecCtx: %s\n", error_buffer);
        return 1;
    }

    if (avcodec_parameters_from_context(Env->outputVideoStream->codecpar, Env->outputCodecCtx) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avcodec_parameters_from_context outputVideoStream: %s\n", error_buffer);
        return 1;
    }
    if (avio_open(&Env->outputFmtCtx->pb, outputUrl, AVIO_FLAG_WRITE) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avio_open outputFmtCtx outputUrl: %s\n", error_buffer);
        return 1;
    }
    if (avformat_write_header(Env->outputFmtCtx, NULL) < 0)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "avformat_write_header outputFmtCtx: %s\n", error_buffer);
        return 1;
    }
    Env->OneFrame = av_frame_alloc();
    if (!Env->OneFrame)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "av_frame_alloc OneFrame: %s\n", error_buffer);
        return 1;
    }
    Env->yoloFrame = av_frame_alloc();
    if (!Env->yoloFrame)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "av_frame_alloc yoloFrame: %s\n", error_buffer);
        return 1;
    }
    Env->OnePacket = av_packet_alloc();
    if (!Env->OnePacket)
    {
        av_strerror(ret, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "av_packet_alloc OnePacket: %s\n", error_buffer);
        return 1;
    }
    return 0;
}
void InitSWS(TLibAVEnv *Env)
{
    struct SwsContext *sws_ctx = sws_getContext(
        Env->inputAudioCodecCtx->width,
        Env->inputAudioCodecCtx->height,
        Env->inputAudioCodecCtx->pix_fmt,
        640, 640, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
    uint8_t *rgb_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, 640, 640, 1));
    av_image_fill_arrays(Env->yoloFrame->data,
                         Env->yoloFrame->linesize,
                         rgb_buffer, AV_PIX_FMT_RGB24, 640, 640, 1);
}
void StartStream(TLibAVEnv *Env)
{
    int ret = 0;
    char error_buffer[128];
    while (av_read_frame(Env->inputFmtCtx, Env->OnePacket) >= 0)
    {
        printf("av_read_frame: %ld\n", Env->OnePacket->stream_index);
        if (Env->OnePacket->stream_index == Env->audioInstreamIndex)
        {
            // Audio
            continue;
        }
        if (Env->OnePacket->stream_index == Env->videoInstreamIndex)
        {
            if (avcodec_send_packet(Env->inputVideoCodecCtx, Env->OnePacket) < 0)
            {
                fprintf(stderr, "Error sending packet to decoder\n");
                av_packet_unref(Env->OnePacket);
                continue;
            }
            while (avcodec_receive_frame(Env->inputVideoCodecCtx, Env->OneFrame) >= 0)
            {
                if (avcodec_send_frame(Env->outputCodecCtx, Env->OneFrame) < 0)
                {
                    av_strerror(ret, error_buffer, sizeof(error_buffer));
                    fprintf(stderr, "avcodec_send_frame OneFrame: %s\n", error_buffer);
                    continue;
                }
                while (avcodec_receive_packet(Env->outputCodecCtx, Env->OnePacket) >= 0)
                {
                    AVStream *in_stream = Env->inputFmtCtx->streams[Env->OnePacket->stream_index];
                    AVStream *out_stream = Env->outputFmtCtx->streams[Env->OnePacket->stream_index];

                    Env->OnePacket->pts = av_rescale_q(Env->OnePacket->pts, in_stream->time_base, out_stream->time_base);
                    Env->OnePacket->dts = Env->OnePacket->pts;
                    if (Env->OnePacket->pts < Env->OnePacket->dts)
                    {
                        continue;
                    }
                    if (av_interleaved_write_frame(Env->outputFmtCtx, Env->OnePacket) < 0)
                    {
                        fprintf(stderr, "Error writing packet to output file\n");
                    }
                    av_packet_unref(Env->OnePacket);
                }
            }
        }
        av_packet_unref(Env->OnePacket);
    }
}
void DestroyTLibAVEnv(TLibAVEnv *Env)
{
    av_frame_free(&Env->OneFrame);
    av_frame_free(&Env->yoloFrame);
    av_packet_free(&Env->OnePacket);
    avcodec_free_context(&Env->inputAudioCodecCtx);
    avcodec_free_context(&Env->inputVideoCodecCtx);
    avcodec_free_context(&Env->outputCodecCtx);
    avformat_close_input(&Env->inputFmtCtx);
    if (Env->outputFmtCtx && !(Env->outputFmtCtx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&Env->outputFmtCtx->pb);
    }
    avformat_free_context(Env->outputFmtCtx);
}

#endif