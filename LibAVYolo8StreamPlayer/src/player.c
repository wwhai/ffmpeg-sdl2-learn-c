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
#define PLAYER
#ifdef PLAYER

#include <pthread.h>
#include "libavenv.c"
#include "libsdl2env.c"
#include "queue.c"

enum TStatus
{
    TPlayerStatus_STOP = 0,
    TPlayerStatus_PLAYING,
    TPlayerStatus_ERROR
};
typedef struct
{
    int status;
    TLibAVEnv *AvEnv;
    TLibSDL2Env *Sdl2Env;
    Queue *Queue;
} TPlayer;
TPlayer *NewTPlayer()
{
    TNew(TPlayer, player);
    player->Sdl2Env = NewLibSdl2Env();
    player->AvEnv = NewTLibAVEnv();
    player->Queue = NewQueue();
    return player;
}
void LibAvThreadCallback(void *data)
{
    TPlayer *player = (TPlayer *)data;
    int ret;
    ret = OpenStream(player->AvEnv, "rtsp://192.168.10.244:554/av0_0",
                     "rtmp://192.168.10.163:1935/live/test002");
    if (ret < 0)
    {
        exit(1);
    }
    LibAvStreamEnvLoop(player->AvEnv, player->Queue);
}
void Sdl2ThreadCallback(void *data)
{
    TPlayer *player = (TPlayer *)data;
    int ret;
    ret = InitTLibSDL2Env(player->Sdl2Env, 1920, 1080);
    if (ret < 0)
    {
        exit(1);
    }
    TLibSDL2EnvEventLoop(player->Sdl2Env, player->Queue);
}
void StartTPlayer(TPlayer *player)
{
    void *retival;
    pthread_t Sdl2Thread, LibAvThread;
    pthread_create(&Sdl2Thread, NULL, (void *)&Sdl2ThreadCallback, (void *)player);
    pthread_create(&LibAvThread, NULL, (void *)&LibAvThreadCallback, (void *)player);
    pthread_join(Sdl2Thread, &retival);
    pthread_join(LibAvThread, &retival);
}

void StopTPlayer(TPlayer *player)
{
    if (player->AvEnv)
    {
        DestroyTLibAVEnv(player->AvEnv);
    }
    if (player->Sdl2Env)
    {
        DestroySDL2Env(player->Sdl2Env);
    }
}
#endif