/*////////////////////////////////////////////////////////////////////////////*/

DECLARE_PRIVATE_TYPE(Font);

static void   init_font_system    (void);
static void   quit_font_system    (void);
static Font   font_create         (void* data, nkBool owns_data, nkF32 px_height);
static void   font_destroy        (Font font);
static nkVec2 font_get_text_bounds(Font font, const nkChar* text);
static nkF32  font_get_text_width (Font font, const nkChar* text);
static nkF32  font_get_text_height(Font font, const nkChar* text);
static nkF32  font_get_px_height  (Font font);
static void   font_draw_text      (Font font, nkF32 x, nkF32 y, const nkChar* text, nkVec4 color);
static void   font_draw_char      (Font font, nkF32 x, nkF32 y, nkChar chr,         nkVec4 color);

/*////////////////////////////////////////////////////////////////////////////*/
