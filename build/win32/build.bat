@echo off
setlocal

:: @Setup: Change these names!

if "%~1"=="win32" goto build_win32
if "%~1"=="web" goto build_web
if "%~1"=="tools" goto build_tools

echo please specify build target (win32, web, tools)...
goto end

:build_win32
echo ----------------------------------------

set defs=-D BUILD_NATIVE -D _CRT_SECURE_NO_WARNINGS
set idir=-I ..\..\depends\sdl\win32\include -I ..\..\depends\sdl_mixer\win32\include -I ..\..\depends\freetype\win32\include -I ..\..\depends\nksdk\nklibs -I ..\..\depends\glew\include -I ..\..\depends\glew\source -I ..\..\depends\stb -I ..\..\depends\imgui -I ..\..\source\engine
set ldir=-libpath:..\..\depends\sdl\win32\lib -libpath:..\..\depends\sdl_mixer\win32\lib -libpath:..\..\depends\freetype\win32\lib
set libs=SDL2main.lib SDL2.lib SDL2_mixer.lib freetype.lib d3d11.lib d3dcompiler.lib shell32.lib
set cflg=-std:c++14 -Zc:__cplusplus -W4 -wd4201 -wd4100 -wd4505
set lflg=-incremental:no -ignore:4099

if "%~2"=="release" (
    set cflg=%cflg% -O2
    set lflg=%lflg% -release -subsystem:windows

    if exist tools\packer.exe tools\packer.exe
) else (
    set defs=%defs% -D BUILD_DEBUG -D NK_DEBUG -D SDL_MAIN_HANDLED
    set cflg=%cflg% -Z7

    if exist ..\..\binary\win32\assets.npak del ..\..\binary\win32\assets.npak
)

if not exist ..\..\binary\win32 mkdir ..\..\binary\win32

copy ..\..\depends\sdl_mixer\win32\bin\*.dll ..\..\binary\win32\ > NUL
copy ..\..\depends\sdl\win32\bin\*.dll ..\..\binary\win32\ > NUL

..\..\depends\makeicon\binaries\win32\makeicon -platform:win32 -sizes:256,128,96,64,48,32,16 -input:..\..\redist\win32\files.txt ..\..\redist\win32\res\icon.ico

pushd ..\..\binary\win32
rc -i ..\..\redist\win32\res ..\..\redist\win32\res\icon.rc
call ..\..\build\win32\timer.bat "cl ..\..\source\myapp.cpp %cflg% %defs% %idir% -Fe:game.exe -link %lflg% %ldir% %libs% ..\..\redist\win32\res\icon.res"
popd

echo ----------------------------------------

goto end

:build_web
echo ----------------------------------------

call ..\..\depends\emsdk\emsdk install latest
call ..\..\depends\emsdk\emsdk activate latest
call ..\..\depends\emsdk\emsdk_env.bat

set defs=-D BUILD_WEB
set idir=-I ..\..\depends\nksdk\nklibs -I ..\..\depends\stb -I ..\..\depends\imgui -I ..\..\source\engine
set libs=-s WASM=1 -s USE_SDL=2 -s USE_SDL_MIXER=2 -s USE_OGG=1 -s USE_VORBIS=1 -s USE_FREETYPE=1 -s MIN_WEBGL_VERSION=1 -s MAX_WEBGL_VERSION=2 -lidbfs.js
set cflg=-std=c++14
set lflg=--preload-file ..\..\assets -s EXPORTED_FUNCTIONS="['_main', '_filesystem_ready']" -s EXPORTED_RUNTIME_METHODS="['ccall', 'UTF8ToString']" -s ALLOW_MEMORY_GROWTH

if "%~2"=="release" (
    set cflg=%cflg% -O2
) else (
    set defs=%defs% -D BUILD_DEBUG -D NK_DEBUG
    set cflg=%cflg% -g
    set lfgl=%lflg% -s ASSERTIONS=1 -s SAFE_HEAP=1
)

if not exist ..\..\binary\web mkdir ..\..\binary\web

pushd ..\..\binary\web
call ..\..\build\win32\timer.bat "emcc %libs% %idir% %cflg% %lflg% %defs% ..\..\source\myapp.cpp -o game.html"
popd

echo ----------------------------------------
goto end

:build_tools
echo ----------------------------------------

if not exist ..\..\tools mkdir ..\..\tools

pushd ..\..\tools
cl ..\source\tools\packer.cpp -I ..\depends\nksdk\nklibs -I ..\depends\stb -Fe:packer.exe
del *.obj
popd

echo ----------------------------------------
goto end

:end
endlocal
