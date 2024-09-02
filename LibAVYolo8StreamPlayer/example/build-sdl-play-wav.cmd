rem Copyright (C) 2024 wwhai
rem
rem This program is free software: you can redistribute it and/or modify
rem it under the terms of the GNU Affero General Public License as
rem published by the Free Software Foundation, either version 3 of the
rem License, or (at your option) any later version.
rem
rem This program is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU Affero General Public License for more details.
rem
rem You should have received a copy of the GNU Affero General Public License
rem along with this program.  If not, see <https://www.gnu.org/licenses/>.

gcc sdl-play-wav-demo.c ^
-I../../SDL2-2.30.1/x86_64-w64-mingw32/include ^
-L../../SDL2-2.30.1/x86_64-w64-mingw32/lib ^
-lSDL2main ^
-lSDL2 ^
-lm ^
-lz ^
-o sdl-play-wav-demo
@echo off
echo sdl-play-wav-demo Compilation completed.
