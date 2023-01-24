/*////////////////////////////////////////////////////////////////////////////*/

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#define NK_PRINT_IMPLEMENTATION
#define NK_FILESYS_IMPLEMENTATION
#define NK_NPAK_IMPLEMENTATION
#define NK_STATIC

#define GLEW_STATIC

#if defined(BUILD_WEB)
#include <emscripten.h>
#endif // BUILD_WEB

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <float.h>

#include <nk_define.h>
#include <nk_math.h>
#include <nk_filesys.h>
#include <nk_npak.h>
#include <nk_defer.h>
#include <nk_print.h>
#include <nk_hashmap.h>
#include <nk_hashset.h>
#include <nk_array.h>
#include <nk_string.h>

#include <stb_image.h>

#include <SDL.h>
#include <SDL_mixer.h>

#if defined(BUILD_NATIVE)
#include <glew.c>
#endif // BUILD_NATIVE
#if defined(BUILD_WEB)
#include <GLES2/gl2.h>
#endif // BUILD_WEB

#include "utility.hpp"
#include "audio.hpp"
#include "platform.hpp"
#include "input.hpp"

static void game_init(void);
static void game_quit(void);
static void game_tick(nkF32 dt);
static void game_draw(void);

#include "utility.cpp"
#include "audio.cpp"
#include "platform.cpp"
#include "input.cpp"

static void game_init(void)
{
    // @Incomplete: ...
}

static void game_quit(void)
{
    // @Incomplete: ...
}

static void game_tick(nkF32 dt)
{
    // @Incomplete: ...
}

static void game_draw(void)
{
    // @Incomplete: ...
}

/*////////////////////////////////////////////////////////////////////////////*/
