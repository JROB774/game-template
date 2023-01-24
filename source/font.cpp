/*////////////////////////////////////////////////////////////////////////////*/

static NKCONSTEXPR nkS32 FONT_ATLAS_SIZE = 1024;

DEFINE_PRIVATE_TYPE(Font)
{
    Texture          atlas;
    stbtt_bakedchar* chars;
    nkU8*            data_buffer;
    nkBool           owns_data;
    nkF32            px_height;
};

static Shader g_textshader;

static void init_font_system(void)
{
    g_textshader = load_asset_shader("text.shader");
}

static void quit_font_system(void)
{
    shader_destroy(g_textshader);
}

static Font font_create(void* data, nkBool owns_data, nkF32 px_height)
{
    Font font = ALLOCATE_PRIVATE_TYPE(Font);
    if(!font)
        fatal_error("Failed to allocate font!");

    font->data_buffer = NK_CAST(nkU8*, data);
    font->owns_data = owns_data;

    font->chars = NK_MALLOC_TYPES(stbtt_bakedchar, 96);
    if(!font->chars)
        fatal_error("Failed to allocate chars for font!");

    nkU8* pixels = NK_MALLOC_TYPES(nkU8, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE);
    if(!pixels)
        fatal_error("Failed to allocate pixels for font!");

    stbtt_BakeFontBitmap(font->data_buffer, 0, px_height, pixels, FONT_ATLAS_SIZE,FONT_ATLAS_SIZE, 32,96, font->chars);

    font->atlas = texture_create(FONT_ATLAS_SIZE,FONT_ATLAS_SIZE, 1, pixels, SamplerFilter_Linear, SamplerWrap_Clamp);
    font->px_height = px_height;

    NK_FREE(pixels);

    return font;
}

static void font_destroy(Font font)
{
    if(!font) return;
    texture_destroy(font->atlas);
    if(font->owns_data)
        NK_FREE(font->data_buffer);
    NK_FREE(font);
}

static stbtt_aligned_quad font_get_glyph_quad(Font font, nkChar c, nkF32* x, nkF32* y)
{
    NK_ASSERT(font);

    NK_ASSERT(x);
    NK_ASSERT(y);

    nkS32 w = NK_CAST(nkS32, texture_get_width(font->atlas));
    nkS32 h = NK_CAST(nkS32, texture_get_height(font->atlas));

    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(font->chars, w,h, c-32, x,y,&q, 1);

    return q;
}

static nkVec2 font_get_text_bounds(Font font, const nkChar* text)
{
    NK_ASSERT(font);

    nkF32 x = 0.0f;
    nkF32 y = 0.0f;

    nkF32 x0 = 0.0f;
    nkF32 y0 = 0.0f;
    nkF32 x1 = 0.0f;
    nkF32 y1 = 0.0f;

    for(nkU64 i=0,n=strlen(text); i<n; ++i)
    {
        nkChar c = text[i];
        if(NK_CAST(nkU8,c) >= 32 && NK_CAST(nkU8,c) < 128)
        {
            stbtt_aligned_quad q = font_get_glyph_quad(font, c, &x,&y);

            x0 = nk_min(x0, q.x0);
            y0 = nk_min(y0, q.y0);
            x1 = nk_max(x1, q.x1);
            y1 = nk_max(y1, q.y1);
        }
        else if(c == '\n')
        {
            x = 0.0f;
            y += font->px_height;
        }
    }

    nkVec2 bounds;
    bounds.x = x1-x0;
    bounds.y = y1-y0;
    return bounds;
}

static nkF32 font_get_text_width(Font font, const nkChar* text)
{
    NK_ASSERT(font);
    return font_get_text_bounds(font, text).x;
}

static nkF32 font_get_text_height(Font font, const nkChar* text)
{
    NK_ASSERT(font);
    return font_get_text_bounds(font, text).y;
}

static nkF32 font_get_px_height(Font font)
{
    NK_ASSERT(font);
    return font->px_height;
}

static void font_draw_text(Font font, nkF32 x, nkF32 y, const nkChar* text, nkVec4 color)
{
    NK_ASSERT(font);
    NK_ASSERT(text);

    nkF32 start_x = x;

    if(strlen(text) == 0)
    {
        return;
    }

    imm_begin(DrawMode_Triangles, font->atlas, g_textshader);
    while(*text)
    {
        if(NK_CAST(nkU8,*text) >= 32 && NK_CAST(nkU8,*text) < 128)
        {
            stbtt_aligned_quad q = font_get_glyph_quad(font, *text, &x,&y);

            imm_vertex({ { q.x0,q.y1 }, { q.s0,q.t1 }, color }); // BL
            imm_vertex({ { q.x0,q.y0 }, { q.s0,q.t0 }, color }); // TL
            imm_vertex({ { q.x1,q.y0 }, { q.s1,q.t0 }, color }); // TR
            imm_vertex({ { q.x1,q.y0 }, { q.s1,q.t0 }, color }); // TR
            imm_vertex({ { q.x1,q.y1 }, { q.s1,q.t1 }, color }); // BR
            imm_vertex({ { q.x0,q.y1 }, { q.s0,q.t1 }, color }); // BL
        }
        else if(*text == '\n')
        {
            x = start_x;
            y += font->px_height;
        }
        ++text;
    }
    imm_end();
}

static void font_draw_char(Font font, nkF32 x, nkF32 y, nkChar chr, nkVec4 color)
{
    nkChar buffer[2] = { chr, '\0' };
    font_draw_text(font, x,y, buffer, color);
}

/*////////////////////////////////////////////////////////////////////////////*/
