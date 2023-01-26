/*////////////////////////////////////////////////////////////////////////////*/

DECLARE_PRIVATE_TYPE(Font);

GLOBAL void   init_font_system    (void);
GLOBAL void   quit_font_system    (void);
GLOBAL Font   font_create         (void* data, nkBool owns_data, nkF32 px_height);
GLOBAL void   font_destroy        (Font font);
GLOBAL nkVec2 font_get_text_bounds(Font font, const nkChar* text);
GLOBAL nkF32  font_get_text_width (Font font, const nkChar* text);
GLOBAL nkF32  font_get_text_height(Font font, const nkChar* text);
GLOBAL nkF32  font_get_px_height  (Font font);
GLOBAL void   font_draw_text      (Font font, nkF32 x, nkF32 y, const nkChar* text, nkVec4 color);
GLOBAL void   font_draw_char      (Font font, nkF32 x, nkF32 y, nkChar chr,         nkVec4 color);

/*////////////////////////////////////////////////////////////////////////////*/
