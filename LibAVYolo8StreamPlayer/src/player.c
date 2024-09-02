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
#ifndef PLAYER
#define PLAYER

#include <pthread.h>
#include "libavenv.c"
#include "queue.c"
#include "types.c"
typedef struct
{
    TLibAVEnv *AvEnv;
    Queue *queue;
} TPlayer;

TPlayer *NewTPlayer()
{
    TNew(TPlayer, player);
    player->AvEnv = NewTLibAVEnv();
    player->queue = NewQueue();
    return player;
}
int TPlayerInit(TPlayer *player)
{
    int ret = -1;
    ret = TLibAVEnvInitInputCodec(player->AvEnv, "rtsp://192.168.10.244:554/av0_0");
    if (ret < 0)
    {
        return ret;
    }
    ret = TLibAVEnvInitOutputCodec(player->AvEnv, "rtmp://192.168.10.163:1935/live/test001");
    if (ret < 0)
    {
        return ret;
    }
    ret = TLibAVEnvInitAvFrame(player->AvEnv);
    if (ret < 0)
    {
        return ret;
    }
    ret = TLibAVEnvInitSWS(player->AvEnv);
    if (ret < 0)
    {
        return ret;
    }

    return 1;
}
void *LibAvThreadCallback(void *data)
{
    TPlayer *player = (TPlayer *)data;
    TLibAVEnvReceiveDisplay(player->AvEnv, player->queue);
    pthread_exit(NULL);
}

void StartTPlayer(TPlayer *player)
{
    void *ret_val;
    pthread_t LibAvThread;

    pthread_create(&LibAvThread, NULL, &LibAvThreadCallback, (void *)player);
    pthread_join(LibAvThread, &ret_val);
}

void StopTPlayer(TPlayer *player)
{
    if (player->AvEnv)
    {
        DestroyTLibAVEnv(player->AvEnv);
    }
}
#endif