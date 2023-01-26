/*////////////////////////////////////////////////////////////////////////////*/

#define STB_IMAGE_IMPLEMENTATION
#define NK_FILESYS_IMPLEMENTATION
#define NK_NPAK_IMPLEMENTATION

#define STB_IMAGE_STATIC
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
#include <nk_array.h>
#include <nk_string.h>
#include <nk_hashmap.h>
#include <nk_defer.h>

#include <stb_image.h>

#include "utility.hpp"
#include "application.hpp"
#include "platform.hpp"
#include "audio.hpp"
#include "input.hpp"
#include "truetype_font.hpp"
#include "render.hpp"
#include "imm_draw.hpp"
#include "asset_manager.hpp"

#include "utility.cpp"
#include "platform.cpp"
#include "audio.cpp"
#include "input.cpp"
#include "truetype_font.cpp"
#include "render.cpp"
#include "imm_draw.cpp"
#include "asset_manager.cpp"

INTERNAL nkF32 g_welcome_angle;

GLOBAL void app_main(AppDesc* desc)
{
    // Nothing...
}

GLOBAL void app_init(void)
{
    // Nothing...
}

GLOBAL void app_quit(void)
{
    // Nothing...
}

GLOBAL void app_tick(nkF32 dt)
{
    PERSISTENT nkF32 timer = 0.0f;
    timer += dt;
    g_welcome_angle = nk_sin_range(-0.25f, 0.25f, timer * 3.0f);
}

GLOBAL void app_draw(void)
{
    Texture texture = asset_manager_load<Texture>("welcome.png");

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkF32 tx = ww * 0.5f;
    nkF32 ty = wh * 0.5f;

    nkF32 scale = wh / (texture_get_height(texture) + 256.0f);

    set_blend_mode(BlendMode_Alpha);
    set_viewport(0.0f,0.0f,ww,wh);

    imm_set_projection(nk_orthographic(0.0f,ww,wh,0.0f));

    clear_screen(NK_V3_MAGENTA);

    imm_texture_ex(texture, tx,ty, scale,scale, g_welcome_angle);
}

/*////////////////////////////////////////////////////////////////////////////*/
