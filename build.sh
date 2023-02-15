args=("$@")

if [ "${args[0]}" == "osx" ]; then
    echo "----------------------------------------"

    defs="-D BUILD_NATIVE"
    idir="-I ../../depends/sdl/osx/SDL2.framework/Headers -I ../../depends/sdl_mixer/osx/SDL2_mixer.framework/Headers -I ../../depends/nksdk/nklibs -I ../../depends/glew/include -I ../../depends/glew/source -I ../../depends/stb"
    ldir="-F ../../depends/sdl/osx -F ../../depends/sdl_mixer/osx"
    libs="-framework OpenGL -framework SDL2 -framework SDL2_mixer"
    cflg="-std=c++14 -rpath @executable_path/Frameworks -g" # @Incomplete: Always generating symbols right now...
    lflg=""

    if [ ! -d "binary/osx" ];
        then mkdir -p "binary/osx"
    fi

    cd "binary/osx"
    g++ $cflg $idir $ldir $libs $lflg $defs ../../source/application.cpp -o game

    # Build the .app package manually.
    if [ -d "game.app" ]; then
        rm -rf game.app
    fi
    if [ -d "game.app.dSYM" ]; then
        rm -rf game.app.dSYM
    fi

    mkdir -p game.app
    mkdir -p game.app/Contents/MacOS/Frameworks
    mkdir -p game.app/Contents/Resources

    mv game game.app/Contents/MacOS/game
    mv game.dSYM game.app.dSYM
    cp -r ../../depends/sdl/osx/SDL2.framework game.app/Contents/MacOS/Frameworks
    cp -r ../../depends/sdl_mixer/osx/SDL2_mixer.framework game.app/Contents/MacOS/Frameworks
    cp -r ../../assets game.app/Contents/Resources/assets

    echo "----------------------------------------"

    exit 0
fi

if [ "${args[0]}" == "web" ]; then
    echo "----------------------------------------"

    depends/emsdk/emsdk install latest
    depends/emsdk/emsdk activate latest
    source depends/emsdk/emsdk_env.sh

    defs="-D BUILD_WEB"
    idir="-I ../../depends/nksdk/nklibs -I ../../depends/stb"
    libs="-s WASM=1 -s USE_SDL=2 -s USE_SDL_MIXER=2 -s USE_OGG=1 -s USE_VORBIS=1 -s USE_FREETYPE=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -lidbfs.js"
    cflg="-std=c++14"
    lflg=$"--preload-file ../../assets -s EXPORTED_FUNCTIONS=_main,_main_callback -s EXPORTED_RUNTIME_METHODS=ccall -s ALLOW_MEMORY_GROWTH"

    if [ ! -d "binary/web" ];
        then mkdir -p "binary/web"
    fi

    cd "binary/web"
    emcc $libs $idir $cflg $lflg $defs ../../source/application.cpp -o game.html

    echo "----------------------------------------"

    exit 0
fi

if [ "${args[0]}" == "tools" ]; then
    echo "----------------------------------------"

    if [ ! -d "tools" ];
        then mkdir -p "tools"
    fi

    cd "tools"
    g++ -std=c++11 ../source/tools/packer.cpp -I ../depends/nksdk/nklibs -I ../depends/stb -o packer

    echo "----------------------------------------"

    exit 0
fi

echo "please specify build target (osx, web, tools)..."
