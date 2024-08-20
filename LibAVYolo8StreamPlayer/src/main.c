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
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <signal.h>
#include "player.c"

// 信号处理函数
void signalHandler(int signal)
{
       if (signal == SIGINT)
       {
              printf("Ctrl-C received, stopping the program.\n");
              exit(0);
       }
}
int main()
{
       signal(SIGINT, signalHandler);
       TPlayer *p = NewTPlayer();
       StartTPlayer(p);
       while (1)
       {
#ifdef _WIN32
              Sleep(50);
#else
              sleep(0.05);
#endif
       }
       StopTPlayer(p);
       return 0;
}
