/*////////////////////////////////////////////////////////////////////////////*/

#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#define NK_PRINT_IMPLEMENTATION
#define NK_FILESYS_IMPLEMENTATION
#define NK_NPAK_IMPLEMENTATION

#define STB_IMAGE_STATIC
#define STBTT_STATIC
#define GLEW_STATIC
#define NK_STATIC

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
#include <stb_truetype.h>

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
#include "font.hpp"
#include "render.hpp"
#include "imm_draw.hpp"
#include "assets.hpp"

#include "utility.cpp"
#include "audio.cpp"
#include "platform.cpp"
#include "input.cpp"
#include "font.cpp"
#include "render.cpp"
#include "imm_draw.cpp"
#include "assets.cpp"

static Texture g_welcome_texture;
static nkF32   g_welcome_angle;

static void game_init(void);
static void game_quit(void);
static void game_tick(nkF32 dt);
static void game_draw(void);

static void entry_point(GameDesc* desc)
{
    desc->init = game_init;
    desc->quit = game_quit;
    desc->tick = game_tick;
    desc->draw = game_draw;
}

static void game_init(void)
{
    g_welcome_texture = load_asset_texture("welcome.png", SamplerFilter_Nearest, SamplerWrap_Clamp);
}

static void game_quit(void)
{
    texture_destroy(g_welcome_texture);
}

static void game_tick(nkF32 dt)
{
    static nkF32 timer = 0.0f;
    timer += dt;
    g_welcome_angle = nk_sin_range(-0.25f, 0.25f, timer * 3.0f);
}

static void game_draw(void)
{
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkF32 tx = ww * 0.5f;
    nkF32 ty = wh * 0.5f;

    nkF32 scale = wh / (texture_get_height(g_welcome_texture) + 256.0f);

    set_blend_mode(BlendMode_Alpha);
    set_viewport(0.0f,0.0f,ww,wh);

    imm_set_projection(nk_orthographic(0.0f,ww,wh,0.0f));

    clear_screen(NK_V3_MAGENTA);

    imm_texture_ex(g_welcome_texture, tx,ty, scale,scale, g_welcome_angle);
}

/*////////////////////////////////////////////////////////////////////////////*/
