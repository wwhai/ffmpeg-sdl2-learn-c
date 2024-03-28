# Copyright (C) 2024 wwhai
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
# OBJS specifies which files to compile as part of the project
OBJS = main.c

CC = gcc

INCLUDE_PATHS = -I./clog-0.0.1/include \
                -I./ffmpeg-6.1.1-full_build-shared/include \
                -I./SDL2-2.30.1/x86_64-w64-mingw32/include/SDL2 \
                -I./SDL2_ttf-2.22.0/x86_64-w64-mingw32/include/SDL2 \
                -I./SDL2_image-2.8.2/x86_64-w64-mingw32/include/SDL2 \


LIBRARY_PATHS = -L./clog-0.0.1/lib \
                -L./ffmpeg-6.1.1-full_build-shared/lib \
                -L./SDL2-2.30.1/x86_64-w64-mingw32/lib \
                -L./SDL2_ttf-2.22.0/x86_64-w64-mingw32/lib \
                -L./SDL2_image-2.8.2/x86_64-w64-mingw32/lib \


LINKER_FLAGS = -lSDL2main -lclog -lSDL2 -lSDL2_ttf -lSDL2_ttf -lSDL2_image \
               -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -lm -lz

OBJ_NAME = player-learn


all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS) -o $(OBJ_NAME) -lm
