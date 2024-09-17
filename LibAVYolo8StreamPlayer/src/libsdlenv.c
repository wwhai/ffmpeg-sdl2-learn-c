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
#ifndef SDL2ENV
#define SDL2ENV

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <libavformat/avformat.h>

#include "types.c"
#include "queue.c"
#include <pthread.h>

/// @brief
typedef struct TLibSdlEnv
{
    SDL_Window *mainWindow;
    SDL_Renderer *mainRenderer;
    SDL_Texture *mainTexture;
    TTF_Font *mainFont;

} TLibSdlEnv;
/// @brief
/// @param Env
void DestroySDL2Env(TLibSdlEnv *Env);
/// @brief
/// @param Env
/// @param w
/// @param h
/// @return
int InitTLibSdlEnv(TLibSdlEnv *Env, int w, int h);
/// @brief
/// @param Env
/// @param queue
void TLibSdlEnvEventLoop(TLibSdlEnv *Env, Queue *queue);
/// @brief
/// @param Env
/// @param sdl_frame
void TLibSdlEnvDisplayFrame(TLibSdlEnv *Env, AVFrame *sdl_frame);
/// @brief
/// @param Env
/// @param text
/// @param x
/// @param y
/// @return
int TLibSdlEnvDrawText(TLibSdlEnv *Env, const char *text, int x, int y);

TLibSdlEnv *NewLibSdl2Env()
{
    TNew(TLibSdlEnv, Sdl2Env);
    return Sdl2Env;
}
void TLibSdlDrawBox(TLibSdlEnv *Env, const char *text, int x, int y, int w, int h, int thickness)
{
    SDL_SetRenderDrawColor(Env->mainRenderer, 255, 0, 0, 0);
    SDL_Rect top = {x, y, w, thickness};
    SDL_RenderFillRect(Env->mainRenderer, &top);
    SDL_Rect bottom = {x, y + h - thickness, w, thickness};
    SDL_RenderFillRect(Env->mainRenderer, &bottom);
    SDL_Rect left = {x, y, thickness, h};
    SDL_RenderFillRect(Env->mainRenderer, &left);
    SDL_Rect right = {x + w - thickness, y, thickness, h};
    SDL_RenderFillRect(Env->mainRenderer, &right);
    if (text != NULL)
    {
        SDL_Color textColor = {255, 0, 0, 0};
        SDL_Surface *textSurface = TTF_RenderText_Solid(Env->mainFont, text, textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(Env->mainRenderer, textSurface);
        SDL_Rect textRect;
        textRect.x = x;
        textRect.y = y - textSurface->h;
        textRect.w = textSurface->w;
        textRect.h = textSurface->h;
        SDL_RenderCopy(Env->mainRenderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}
/// @brief
/// @param Env
/// @param text
/// @param x
/// @param y
/// @return
int TLibSdlEnvDrawText(TLibSdlEnv *Env, const char *text, int x, int y)
{
    SDL_Color color = {255, 0, 0, 255};
    SDL_Surface *textSurface = TTF_RenderText_Solid(Env->mainFont, text, color);
    if (textSurface == NULL)
    {
        SDL_Log("Unable to create text surface: %s", TTF_GetError());
        return -1;
    }
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(Env->mainRenderer, textSurface);
    if (textTexture == NULL)
    {
        SDL_Log("Unable to create texture from text surface: %s", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return -1;
    }
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(Env->mainRenderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
    return 0;
}
/// @brief
/// @param Env
/// @param sdl_frame
void TLibSdlEnvDisplayFrame(TLibSdlEnv *Env, AVFrame *sdl_frame)
{
    if (sdl_frame == NULL)
    {
        printf("TLibSdlEnvDisplayFrame == NULL\n");
        return;
    }

    SDL_UpdateYUVTexture(Env->mainTexture, NULL,
                         sdl_frame->data[0], sdl_frame->linesize[0],
                         sdl_frame->data[1], sdl_frame->linesize[1],
                         sdl_frame->data[2], sdl_frame->linesize[2]);
    SDL_Rect srcRect = {0, 0, 1920, 1080};
    SDL_Rect distRect = {0, 0, 1920 / 2, 1080 / 2};
    SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, &srcRect, &distRect);
}
/// @brief 初始化
/// @param Env
/// @param w
/// @param h
/// @return
int InitTLibSdlEnv(TLibSdlEnv *Env, int w, int h)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO |
                 SDL_INIT_TIMER | SDL_INIT_EVENTS))
    {
        printf("SDL_Init Error: %s\n", TTF_GetError());
        return -1;
    }

    Env->mainWindow = SDL_CreateWindow("YOLO8_Object_Detection",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       w, h, SDL_WINDOW_SHOWN);
    if (!Env->mainWindow)
    {
        printf("SDL_CreateWindow Error: %s\n", TTF_GetError());
        return -1;
    }
    Env->mainRenderer = SDL_CreateRenderer(Env->mainWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!Env->mainRenderer)
    {
        printf("SDL_CreateWindow Error: %s\n", TTF_GetError());
        return -1;
    }
    Env->mainTexture = SDL_CreateTexture(Env->mainRenderer,
                                         SDL_PIXELFORMAT_IYUV,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         w, h);
    if (!Env->mainTexture)
    {
        printf("SDL_CreateTexture Error: %s\n", TTF_GetError());
        return -1;
    }
    if (TTF_Init() != 0)
    {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }
    Env->mainFont = TTF_OpenFont("font.ttf", 24);
    if (!Env->mainFont)
    {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        return -1;
    }
    return 0;
}
/// @brief 开始显示界面
/// @param Env
/// @param queue
void TLibSdlEnvEventLoop(TLibSdlEnv *Env, Queue *queue)
{
    SDL_Event e;
    int running = 1;
    QueueData data;
    while (running)
    {
        //------------------------------------------------------------------------------------------
        char stringbuf[100];
        while (!isEmpty(queue))
        {
            pthread_mutex_lock(&thread_mutex);
            data = dequeue(queue);
            pthread_mutex_unlock(&thread_mutex);
            sprintf(stringbuf, "Frame: %p, X: %d, Y: %d, Width: %d, Height: %d, Label: %s:%.2f%%",
                    data.frame, 0, 0, data.frame->width, data.frame->height, data.label, data.prop);
            printf("======== LibSdlThreadCallback : %s\n", stringbuf);
        }
        // SDL_RenderClear(Env->mainRenderer);
        if (data.frame != NULL)
        {
            SDL_RenderClear(Env->mainRenderer);
            SDL_UpdateYUVTexture(Env->mainTexture, NULL,
                                 data.frame->data[0], data.frame->linesize[0],
                                 data.frame->data[1], data.frame->linesize[1],
                                 data.frame->data[2], data.frame->linesize[2]);
            SDL_Rect srcRect = {0, 0, 1920, 1080};
            SDL_Rect distRect1 = {0, 0, 1920 / 2, 1080 / 2};
            SDL_Rect distRect2 = {1920 / 2, 0, 1920 / 2, 1080 / 2};

            SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, &srcRect, &distRect1);
            SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, &srcRect, &distRect2);
            SDL_RenderPresent(Env->mainRenderer);
            // SDL_UpdateYUVTexture(Env->mainTexture, NULL,
            //                      data.frame->data[0], data.frame->linesize[0],
            //                      data.frame->data[1], data.frame->linesize[1],
            //                      data.frame->data[2], data.frame->linesize[2]);
            // SDL_Rect srcRect = {0, 0, 1920, 1080};
            // SDL_Rect distRect1 = {0, 0, 1920 / 2, 1080 / 2};
            // SDL_Rect distRect2 = {1920 / 2, 0, 1920 / 2, 1080 / 2};
            // SDL_Rect distRect3 = {0, 1080 / 2, 1920 / 2, 1080 / 2};
            // SDL_Rect distRect4 = {1920 / 2, 1080 / 2, 1920 / 2, 1080 / 2};
            // SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, &srcRect, &distRect1);
            // SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, &srcRect, &distRect2);
            // SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, &srcRect, &distRect3);
            // SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, &srcRect, &distRect4);
        }
        // SDL_RenderPresent(Env->mainRenderer);
        //------------------------------------------------------------------------------------------

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = 0;
                }
            }
            if ((SDL_GetTicks() - SDL_GetTicks()) < 10)
            {
                SDL_Delay(10);
            }
        }
    }
    DestroySDL2Env(Env);
}
void DestroySDL2Env(TLibSdlEnv *Env)
{
    SDL_DestroyTexture(Env->mainTexture);
    SDL_DestroyRenderer(Env->mainRenderer);
    SDL_DestroyWindow(Env->mainWindow);
    TTF_Quit();
    SDL_Quit();
}

#endif