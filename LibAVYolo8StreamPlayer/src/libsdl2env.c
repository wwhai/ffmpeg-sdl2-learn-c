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
#define SDL2ENV
#ifdef SDL2ENV

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <libavformat/avformat.h>
#include "types.c"

typedef struct TLibSDL2Env
{
    SDL_Window *mainWindow;
    SDL_Renderer *mainRenderer;
    SDL_Texture *mainTexture;
    TTF_Font *mainFont;
} TLibSDL2Env;

void TLibSDL2EnvDrawBox(TLibSDL2Env *Env, const char *text,
                        int x, int y, int w, int h, int thickness)
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
        SDL_Color textColor = {255, 0, 0, 0}; // RGBA
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
void TLibSDL2EnvDisplayFrame(TLibSDL2Env *Env, AVFrame *sdl_frame)
{
    SDL_RenderClear(Env->mainRenderer);
    SDL_UpdateYUVTexture(Env->mainTexture, NULL,
                         sdl_frame->data[0], sdl_frame->linesize[0],
                         sdl_frame->data[1], sdl_frame->linesize[1],
                         sdl_frame->data[2], sdl_frame->linesize[2]);
    SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, NULL, NULL);
    SDL_RenderPresent(Env->mainRenderer);
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
    Env->mainWindow = SDL_CreateWindow("FSY PLAYER", 0, 0, w, h, SDL_WINDOW_SHOWN);
    if (!Env->mainWindow)
    {
        printf("SDL_CreateWindow Error: %s\n", TTF_GetError());
        return -1;
    }
    Env->mainRenderer = SDL_CreateRenderer(Env->mainWindow, -1, 0);
    if (!Env->mainRenderer)
    {
        printf("SDL_CreateWindow Error: %s\n", TTF_GetError());
        return -1;
    }
    Env->mainTexture = SDL_CreateTexture(Env->mainRenderer,
                                         SDL_PIXELFORMAT_YV12,
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
void TLibSDL2EnvEventLoop(TLibSDL2Env *Env)
{
    SDL_Event e;
    int running = 1;
    int mouse_x = 0, mouse_y = 0;
    while (running)
    {
        SDL_RenderClear(Env->mainRenderer);
        TLibSDL2EnvDrawBox(Env, "CLASS:A", mouse_x - 10, mouse_y - 10, 100, 100, 5);
        // SDL_RenderCopy(Env->mainRenderer, Env->mainTexture, NULL, NULL);
        SDL_RenderPresent(Env->mainRenderer);

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
void DestroySDL2Env(TLibSDL2Env *Env)
{
    SDL_DestroyTexture(Env->mainTexture);
    SDL_DestroyRenderer(Env->mainRenderer);
    SDL_DestroyWindow(Env->mainWindow);
    TTF_Quit();
    SDL_Quit();
}

#endif