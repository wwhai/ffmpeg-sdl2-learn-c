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
#if defined(_WIN32) || defined(_WIN64)
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>

void DrawBox(SDL_Renderer *renderer, TTF_Font *font, const char *text,
             int x, int y, int w, int h, int thickness)
{
    // Draw the rectangle with the specified thickness
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0); // White color

    // Draw top border
    SDL_Rect top = {x, y, w, thickness};
    SDL_RenderFillRect(renderer, &top);

    // Draw bottom border
    SDL_Rect bottom = {x, y + h - thickness, w, thickness};
    SDL_RenderFillRect(renderer, &bottom);

    // Draw left border
    SDL_Rect left = {x, y, thickness, h};
    SDL_RenderFillRect(renderer, &left);

    // Draw right border
    SDL_Rect right = {x + w - thickness, y, thickness, h};
    SDL_RenderFillRect(renderer, &right);

    // Render text above the rectangle
    if (text != NULL)
    {
        SDL_Color textColor = {255, 0, 0, 0}; // White color
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect;
        textRect.x = x;
        textRect.y = y - textSurface->h; // Position text above the rectangle
        textRect.w = textSurface->w;
        textRect.h = textSurface->h;

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

int main(int argc, char *argv[])
{
    const char *video_file = "rtsp://192.168.10.244:554/av0_0";

    AVFormatContext *format_ctx = avformat_alloc_context();
    if (avformat_open_input(&format_ctx, video_file, NULL, NULL) != 0)
    {
        fprintf(stderr, "Could not open video file.\n");
        return -1;
    }

    if (avformat_find_stream_info(format_ctx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream information.\n");
        return -1;
    }

    int video_stream_index = -1;
    for (int i = 0; i < format_ctx->nb_streams; i++)
    {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1)
    {
        fprintf(stderr, "Could not find video stream.\n");
        return -1;
    }

    AVCodecParameters *codec_params = format_ctx->streams[video_stream_index]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec)
    {
        fprintf(stderr, "Could not find codec.\n");
        return -1;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codec_ctx, codec_params) < 0)
    {
        fprintf(stderr, "Could not copy codec context.\n");
        return -1;
    }

    if (avcodec_open2(codec_ctx, codec, NULL) < 0)
    {
        fprintf(stderr, "Could not open codec.\n");
        return -1;
    }

    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    if (TTF_Init() != 0)
    {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    int width = codec_ctx->width;
    int height = codec_ctx->height;

    SDL_Window *window = SDL_CreateWindow("FFmpeg + SDL2 Video",
                                          0, 0,
                                          width, height,
                                          SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "SDL: could not create window - %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        fprintf(stderr, "SDL: could not create renderer - %s\n", SDL_GetError());
        return -1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_YV12,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             width, height);
    if (!texture)
    {
        fprintf(stderr, "SDL: could not create texture - %s\n", SDL_GetError());
        return -1;
    }
    TTF_Font *font = TTF_OpenFont("Ubuntu-Title.ttf", 24);
    if (font == NULL)
    {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    struct SwsContext *sws_ctx = sws_getContext(width, height, codec_ctx->pix_fmt,
                                                width, height, AV_PIX_FMT_YUV420P,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx)
    {
        fprintf(stderr, "Cannot initialize the conversion context!\n");
        return -1;
    }

    AVFrame *frame = av_frame_alloc();
    AVFrame *sdl_frame = av_frame_alloc();
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 32);
    uint8_t *buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(sdl_frame->data, sdl_frame->linesize, buffer, AV_PIX_FMT_YUV420P, width, height, 32);

    AVPacket packet;
    SDL_Event e;
    int running = 1;
    int mouse_x = 0, mouse_y = 0;
    while (running)
    {
        while (av_read_frame(format_ctx, &packet) >= 0)
        {
            if (packet.stream_index == video_stream_index)
            {
                int ret = avcodec_send_packet(codec_ctx, &packet);
                if (ret < 0)
                {
                    fprintf(stderr, "Error sending packet for decoding.\n");
                    continue;
                }

                ret = avcodec_receive_frame(codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    continue;
                }
                else if (ret < 0)
                {
                    fprintf(stderr, "Error during decoding.\n");
                    break;
                }

                sws_scale(sws_ctx, (uint8_t const *const *)frame->data,
                          frame->linesize, 0, height,
                          sdl_frame->data, sdl_frame->linesize);

                SDL_RenderClear(renderer);
                SDL_UpdateYUVTexture(texture, NULL,
                                     sdl_frame->data[0], sdl_frame->linesize[0],
                                     sdl_frame->data[1], sdl_frame->linesize[1],
                                     sdl_frame->data[2], sdl_frame->linesize[2]);

                SDL_RenderCopy(renderer, texture, NULL, NULL);
                // 画框
                DrawBox(renderer, font, "CLASS:A", mouse_x - 10, mouse_y - 10, 100, 100, 5);
                SDL_RenderPresent(renderer);
            }
            av_packet_unref(&packet);
            // SDL EVENT
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT)
                {
                    running = 0;
                }
                else if (e.type == SDL_MOUSEMOTION)
                {
                    mouse_x = e.motion.x;
                    mouse_y = e.motion.y;
                }
                else if (e.type == SDL_KEYDOWN)
                {
                    if (e.key.keysym.sym == SDLK_ESCAPE)
                    {
                        running = 0;
                    }
                }
            }
            SDL_Delay(30);
        }
    }

    av_frame_free(&frame);
    av_frame_free(&sdl_frame);
    av_free(buffer);
    sws_freeContext(sws_ctx);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    avcodec_close(codec_ctx);
    avformat_close_input(&format_ctx);

    return 0;
}
