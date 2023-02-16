/*////////////////////////////////////////////////////////////////////////////*/

#define STB_IMAGE_IMPLEMENTATION
#define NK_FILESYS_IMPLEMENTATION
#define NK_NPAK_IMPLEMENTATION

#define STB_IMAGE_STATIC
#define GLEW_STATIC
#define NK_STATIC

#define USE_RENDERER_SIMPLE

#include <nk_define.h>
#include <nk_math.h>
#include <nk_filesys.h>
#include <nk_npak.h>
#include <nk_array.h>
#include <nk_string.h>
#include <nk_hashmap.h>
#include <nk_defer.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <wchar.h>

#include <stb_image.h>

#include "utility.hpp"
#include "collision.hpp"
#include "application.hpp"
#include "platform.hpp"
#include "asset_manager.hpp"
#include "audio.hpp"
#include "input.hpp"
#include "bitmap_font.hpp"
#include "truetype_font.hpp"
#include "animation.hpp"
#include "renderer.hpp"
#include "assets.hpp"

#include "utility.cpp"
#include "collision.cpp"
#include "platform.cpp"
#include "asset_manager.cpp"
#include "audio.cpp"
#include "input.cpp"
#include "bitmap_font.cpp"
#include "truetype_font.cpp"
#include "animation.cpp"
#include "renderer.cpp"

INTERNAL constexpr nkF32 MIN_FACE_ROTATION = -0.30f;
INTERNAL constexpr nkF32 MAX_FACE_ROTATION =  0.30f;
INTERNAL constexpr nkF32 ROTATION_SPEED    =  5.00f;
INTERNAL constexpr nkF32 SHADOW_OFFSET_X   =  8.00f;
INTERNAL constexpr nkF32 SHADOW_OFFSET_Y   =  8.00f;

INTERNAL nkF32 g_face_timer;
INTERNAL nkF32 g_face_angle;

GLOBAL void app_main(AppDesc* desc)
{
    // Nothing...
}

GLOBAL void app_init(void)
{
    TrueTypeFontDesc ttd;
    ttd.px_sizes = { 12, 100 };
    ttd.flags   |= TrueTypeFontFlags_Monochrome;
    asset_manager_load<TrueTypeFont>("helsinki.ttf", &ttd);
}

GLOBAL void app_quit(void)
{
    // Nothing...
}

GLOBAL void app_tick(nkF32 dt)
{
    g_face_timer += dt;

    g_face_angle = nk_sin_range(MIN_FACE_ROTATION, MAX_FACE_ROTATION, g_face_timer * ROTATION_SPEED);
    g_face_angle = nk_clamp(g_face_angle * 2.5f, MIN_FACE_ROTATION, MAX_FACE_ROTATION);
}

GLOBAL void app_draw(void)
{
    TrueTypeFont font = asset_manager_load<TrueTypeFont>("helsinki.ttf");
    Texture face = asset_manager_load<Texture>("face.png");

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkF32 hw = ww * 0.5f;
    nkF32 hh = wh * 0.5f;

    nkF32 scale = wh / (get_texture_height(face) + 512.0f);

    clear_screen(0.0f, 0.6f, 0.8f);

    imm_set_projection(nk_orthographic(0.0f,ww,wh,0.0f));
    imm_set_viewport({ 0.0f,0.0f,ww,wh });

    imm_texture_ex(face, hw,hh, scale,scale, g_face_angle, NULL);

    const nkChar* text = "Hello, World!";

    set_truetype_font_size(font, 100);

    nkF32 tx0 = (ww - get_truetype_text_width(font, text)) * 0.5f;
    nkF32 ty0 = (hh - (get_texture_height(face) * scale * 0.6f));

    nkF32 tx1 = (ww - get_truetype_text_width(font, text)) * 0.5f;
    nkF32 ty1 = (hh + (get_texture_height(face) * scale * 0.6f) + (get_truetype_line_height(font) * 0.5f));

    draw_truetype_text(font, tx0+SHADOW_OFFSET_X,ty0+SHADOW_OFFSET_Y, text, NK_V4_BLACK);
    draw_truetype_text(font, tx0,ty0, text, NK_V4_WHITE);

    draw_truetype_text(font, tx1+SHADOW_OFFSET_X,ty1+SHADOW_OFFSET_Y, text, NK_V4_BLACK);
    draw_truetype_text(font, tx1,ty1, text, NK_V4_WHITE);
}

/*////////////////////////////////////////////////////////////////////////////*/
