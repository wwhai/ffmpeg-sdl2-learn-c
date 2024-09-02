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
#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// 音频回调函数
void audio_callback(void *userdata, Uint8 *stream, int len)
{
    // 检查是否有音频数据可以播放
    if (SDL_GetAudioStatus() != SDL_AUDIO_PLAYING)
    {
        SDL_LockAudio();
        SDL_QueueAudio(0, stream, len);
        SDL_UnlockAudio();
    }
}

int main(int argc, char *args[])
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // 打开音频设备
    SDL_AudioSpec desired_spec, obtained_spec;
    SDL_zero(desired_spec);
    desired_spec.freq = 44100;          // 采样率
    desired_spec.format = AUDIO_S16LSB; // 16位有符号整数
    desired_spec.channels = 2;          // 声道数（立体声）
    desired_spec.samples = 1024;        // 缓冲区大小
    desired_spec.callback = audio_callback;
    desired_spec.userdata = NULL;

    if (SDL_OpenAudio(&desired_spec, &obtained_spec) < 0)
    {
        SDL_Log("SDL could not open audio! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // 加载WAV文件
    SDL_AudioSpec wav_spec;
    Uint8 *wav_buffer;
    Uint32 wav_length;
    if (SDL_LoadWAV("./wav1.wav", &wav_spec, &wav_buffer, &wav_length) == NULL)
    {
        SDL_Log("Unable to load WAV file! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // 开始播放音频
    SDL_PauseAudio(0);

    // 播放WAV文件
    SDL_QueueAudio(0, wav_buffer, wav_length);

    // 等待音频播放完毕
    while (SDL_GetAudioStatus() == SDL_AUDIO_PLAYING)
    {
        SDL_Delay(100);
    }

    // 清理资源
    SDL_FreeWAV(wav_buffer);
    SDL_CloseAudio();
    SDL_Quit();

    return 0;
}
