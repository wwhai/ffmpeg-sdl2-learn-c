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

typedef struct TLibSDL2Env
{
    SDL_Window *mainWindow;
    SDL_Renderer *mainRenderer;
    SDL_Texture *mainTexture;
    TTF_Font *mainFont;
} TLibSDL2Env;
void DestroySDL2Env(TLibSDL2Env *Env);
int SDLDrawText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y);
void SDLDrawRect(SDL_Renderer *renderer, int x, int y, int width, int height);
void TLibSDL2EnvDisplayFrame(TLibSDL2Env *Env, AVFrame *sdl_frame);
// 封装的函数，用于渲染文本

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

int SDLDrawText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y)
{

    SDL_Color color = {255, 0, 0, 255}; // RGBA
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    if (textSurface == NULL)
    {
        SDL_Log("Unable to create text surface: %s", TTF_GetError());
        return -1; // 返回错误代码
    }
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == NULL)
    {
        SDL_Log("Unable to create texture from text surface: %s", SDL_GetError());
        SDL_FreeSurface(textSurface); // 清理表面
        return -1;                    // 返回错误代码
    }
    SDL_Rect textRect = {x, y - 20, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);

    return 0; // 成功返回0
}
void SDLDrawRect(SDL_Renderer *renderer, int x, int y,
                 int width, int height)
{
    SDL_Color color = {255, 0, 0, 255}; // RGBA
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderDrawRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}
void TLibSDL2EnvDisplayFrame(TLibSDL2Env *Env, AVFrame *sdl_frame)
{
    SDL_UpdateYUVTexture(Env->mainTexture, NULL,
                         sdl_frame->data[0], sdl_frame->linesize[0],
                         sdl_frame->data[1], sdl_frame->linesize[1],
                         sdl_frame->data[2], sdl_frame->linesize[2]);
    SDL_Rect srcRect = {0, 0, 1920, 1080};
    SDL_Rect distRect = {0, 0, 1920, 1080};
    SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, &srcRect, &distRect);
}

TLibSDL2Env *NewLibSdl2Env()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO |
                 SDL_INIT_TIMER | SDL_INIT_EVENTS))
    {
        printf("SDL_Init Error: %s\n", TTF_GetError());
        return NULL;
    }
    if (TTF_Init() != 0)
    {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return NULL;
    }
    TNew(TLibSDL2Env, Sdl2Env);
    return Sdl2Env;
}
int InitTLibSDL2Env(TLibSDL2Env *Env, int w, int h)
{
    Env->mainWindow = SDL_CreateWindow("FSY PLAYER",
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
    Env->mainFont = TTF_OpenFont("Duran-Medium.ttf", 24);
    if (!Env->mainFont)
    {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        return -1;
    }
    return 0;
}
void TLibSDL2EnvEventLoop(TLibSDL2Env *Env, Queue *queue)
{
    SDL_Event e;
    int running = 1;
    int mouse_x = 0, mouse_y = 0;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    QueueData oqd;
    while (running)
    {

        //------------------------------------------------------------------------------------------
        SDL_RenderClear(Env->mainRenderer);
        SDL_SetRenderDrawColor(Env->mainRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        if (!isQueueEmpty(queue))
        {
            for (size_t i = 0; i < queueSize(queue); i++)
            {
                pthread_mutex_lock(&lock);
                QueueData nqd = dequeue(queue);
                pthread_mutex_unlock(&lock);
                oqd.frame = nqd.frame;
            }
        }
        if (oqd.frame != NULL)
        {
            TLibSDL2EnvDisplayFrame(Env, oqd.frame);
        }
        SDLDrawText(Env->mainRenderer, Env->mainFont, "Hello!", mouse_x, mouse_y);
        SDLDrawRect(Env->mainRenderer, mouse_x, mouse_y, 150, 100);
        SDL_RenderPresent(Env->mainRenderer);
        //------------------------------------------------------------------------------------------

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
                // printf("SDL_MOUSEMOTION: %d,%d\n", mouse_x, mouse_y);
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
void DestroySDL2Env(TLibSDL2Env *Env)
{
    SDL_DestroyTexture(Env->mainTexture);
    SDL_DestroyRenderer(Env->mainRenderer);
    SDL_DestroyWindow(Env->mainWindow);
    TTF_Quit();
    SDL_Quit();
}

#endif