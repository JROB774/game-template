/*////////////////////////////////////////////////////////////////////////////*/

typedef struct ImmVertex
{
    nkVec2 pos;
    nkVec2 tex;
    nkVec4 col;
}
ImmVertex;

typedef struct ImmRect
{
    nkF32 x,y,w,h;
}
ImmRect;

typedef struct ImmAtlasClip
{
    nkF32   original_width;
    nkF32   original_height;
    nkF32   offset_x;
    nkF32   offset_y;
    ImmRect clip_bounds;
}
ImmAtlasClip;

static void imm_init(void);
static void imm_quit(void);

static nkMat4 imm_get_projection(void);
static nkMat4 imm_get_view      (void);
static nkMat4 imm_get_model     (void);

static void imm_set_projection(nkMat4 projection);
static void imm_set_view      (nkMat4 view);
static void imm_set_model     (nkMat4 model);

static void imm_begin (DrawMode draw_mode, Texture tex, Shader shader);
static void imm_end   (void);
static void imm_vertex(ImmVertex v);

static void imm_point         (nkF32  x, nkF32  y,                     nkVec4 color);
static void imm_line          (nkF32 x1, nkF32 y1, nkF32 x2, nkF32 y2, nkVec4 color);
static void imm_rect_outline  (nkF32  x, nkF32  y, nkF32  w, nkF32  h, nkVec4 color);
static void imm_rect_filled   (nkF32  x, nkF32  y, nkF32  w, nkF32  h, nkVec4 color);
static void imm_circle_outline(nkF32  x, nkF32  y, nkF32  r, nkS32  n, nkVec4 color);
static void imm_circle_filled (nkF32  x, nkF32  y, nkF32  r, nkS32  n, nkVec4 color);

static void imm_begin_texture_batch(Texture tex);
static void imm_end_texture_batch  (void);
static void imm_texture            (Texture tex, nkF32 x, nkF32 y,                                                  const ImmRect*      clip, nkVec4 color);
static void imm_texture_ex         (Texture tex, nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmRect*      clip, nkVec4 color);
static void imm_texture_batched    (             nkF32 x, nkF32 y,                                                  const ImmRect*      clip, nkVec4 color);
static void imm_texture_batched_ex (             nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmRect*      clip, nkVec4 color);

/*////////////////////////////////////////////////////////////////////////////*/
