#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include "clog.h"
#include <time.h>
#include <windows.h>
#include <SDL_syswm.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// 自定义的窗口处理函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        MessageBox(hwnd, "WINDOWS API CLOSE", "INFO", MB_OK);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void exit_hook(void)
{
    printf("Handle exit_hook\n");
}
//
// main
//
int main(int argc, char *argv[])
{
    log_set_level(LOG_DEBUG);
    atexit(exit_hook);
    log_debug("SDL_Init");
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "无法初始化SDL: %s\n", SDL_GetError());
        return 1;
    }

    // 初始化SDL_ttf
    if (TTF_Init() == -1)
    {
        fprintf(stderr, "无法初始化SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("Hello SDL2", 100, 100,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        fprintf(stderr, "无法创建窗口: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    // 获取窗口句柄
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(window, &wmInfo))
    {
        HWND hwnd = wmInfo.info.win.window;

        // 设置自定义的窗口处理函数
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
    }
    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        fprintf(stderr, "无法创建渲染器: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // 加载字体
    TTF_Font *font = TTF_OpenFont("./Ubuntu-Title.ttf", 24);
    if (font == NULL)
    {
        fprintf(stderr, "无法加载字体: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // 设置文字颜色
    SDL_Color textColor = {255, 255, 255, 255};

    // 事件循环标志
    int running = 1;

    // 帧率计算
    Uint32 startTick, frameCount = 0;
    char fpsText[20] = "FPS: 0";

    // 事件循环
    while (running)
    {
        startTick = SDL_GetTicks();
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT: // 窗口关闭事件
                running = 0;
                log_debug("SDL_QUIT");
                continue;
            case SDL_KEYDOWN:
                break;
            default:
                break;
            }
        }
        if ((SDL_GetTicks() - SDL_GetTicks()) < 10)
        {
            SDL_Delay(10);
        }
        // 清除屏幕
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 渲染文字
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, "Hello SDL2", textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {50, 50, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // 渲染帧率
        frameCount++;
        if (frameCount % 60 == 0)
        { // 每60帧更新一次帧率显示
            sprintf(fpsText, "FPS: %d", (int)(1000.0 / (SDL_GetTicks() - startTick + 1)));
        }
        textSurface = TTF_RenderText_Solid(font, fpsText, textColor);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect fpsRect = {10, 10, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &fpsRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
        SDL_RenderPresent(renderer);
    }
    // 清理资源
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}