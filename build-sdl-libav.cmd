@echo off
gcc sdl-libav-demo.c ^
-I./clog-0.0.1/include ^
-I./ffmpeg-6.1.1-full_build-shared/include ^
-I./SDL2-2.30.1/x86_64-w64-mingw32/include ^
-I./SDL2_ttf-2.22.0/x86_64-w64-mingw32/include ^
-I./SDL2_image-2.8.2/x86_64-w64-mingw32/include ^
-L./clog-0.0.1/lib ^
-L./ffmpeg-6.1.1-full_build-shared/lib ^
-L./SDL2-2.30.1/x86_64-w64-mingw32/lib ^
-L./SDL2_ttf-2.22.0/x86_64-w64-mingw32/lib ^
-L./SDL2_image-2.8.2/x86_64-w64-mingw32/lib ^
-lSDL2main ^
-lclog ^
-lSDL2 ^
-lSDL2_ttf ^
-lSDL2_image ^
-lavformat ^
-lavfilter ^
-lavcodec ^
-lswresample ^
-lswscale ^
-lavutil ^
-lm ^
-lz ^
-o sdl-libav-demo ^
-lm

echo Compilation completed.
