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
#include <stdio.h>
#include "player.c"

int main()
{
       TPlayer *p = NewTPlayer();
       TPlayerInit(p);
       StartTPlayer(p);
       while (1)
       {
              printf("Enter a character ('q' to quit): ");
              char inputChar = getchar();
              if (inputChar == 'q' || inputChar == 'Q')
              {
                     printf("Exiting.\n");
                     goto END;
              }
       }
END:
       StopTPlayer(p);
       return 0;
}
