// Copyright (C) 2024 wwhai
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#define PLAYER_H
#ifdef PLAYER_H
typedef unsigned char TBool;

enum
{
    TFalse = 0,
    TTrue = 1
};
enum TStatus
{
    TPlayerStatus_STOP = 0,
    TPlayerStatus_PLAYING,
    TPlayerStatus_ERROR
};
typedef struct
{
    int status;
    char name[32];
} TPlayer;

#endif