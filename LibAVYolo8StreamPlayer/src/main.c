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
#if defined(_WIN32) || defined(_WIN64)
#define SDL_MAIN_HANDLED
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <stdio.h>

int main()
{
    printf("libavcodec version: %d.%d.%d\n",
           (AV_VERSION_MAJOR(avcodec_version())),
           (AV_VERSION_MINOR(avcodec_version())),
           (AV_VERSION_MICRO(avcodec_version())));

    printf("libavformat version: %d.%d.%d\n",
           (AV_VERSION_MAJOR(avformat_version())),
           (AV_VERSION_MINOR(avformat_version())),
           (AV_VERSION_MICRO(avformat_version())));

    printf("libavutil version: %d.%d.%d\n",
           (AV_VERSION_MAJOR(avutil_version())),
           (AV_VERSION_MINOR(avutil_version())),
           (AV_VERSION_MICRO(avutil_version())));

    return 0;
}
