/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(DrawMode, nkS32)
{
    DrawMode_Points,
    DrawMode_LineStrip,
    DrawMode_LineLoop,
    DrawMode_Lines,
    DrawMode_TriangleStrip,
    DrawMode_TriangleFan,
    DrawMode_Triangles
};

NK_ENUM(AttribType, nkS32)
{
    AttribType_SignedByte,
    AttribType_UnsignedByte,
    AttribType_SignedInt,
    AttribType_UnsignedInt,
    AttribType_Float
};

NK_ENUM(BufferType, nkS32)
{
    BufferType_Static,
    BufferType_Dynamic,
    BufferType_Stream
};

NK_ENUM(SamplerFilter, nkS32)
{
    SamplerFilter_Nearest,
    SamplerFilter_Linear,
    SamplerFilter_NearestWithNearestMips,
    SamplerFilter_LinearWithNearestMips,
    SamplerFilter_NearestWithLinearMips,
    SamplerFilter_LinearWithLinearMips
};

NK_ENUM(SamplerWrap, nkS32)
{
    SamplerWrap_Repeat,
    SamplerWrap_Clamp
};

NK_ENUM(BlendMode, nkS32)
{
    BlendMode_None,
    BlendMode_Alpha,
    BlendMode_PremultipliedAlpha
};

static void init_render_system(void);
static void quit_render_system(void);

static void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h);

static void set_blend_mode(BlendMode blend_mode);

static void clear_screen(nkF32 r, nkF32 g, nkF32 b, nkF32 a = 1.0f);
static void clear_screen(nkVec3 color);
static void clear_screen(nkVec4 color);

// VertexBuffer ================================================================
DECLARE_PRIVATE_TYPE(VertexBuffer);

static VertexBuffer vertex_buffer_create        (void);
static void         vertex_buffer_destroy       (VertexBuffer vbuf);
static void         vertex_buffer_set_stride    (VertexBuffer vbuf, nkU64 byte_stride);
static void         vertex_buffer_enable_attrib (VertexBuffer vbuf, nkU32 index, AttribType type, nkU32 comps, nkU64 byte_offset);
static void         vertex_buffer_disable_attrib(VertexBuffer vbuf, nkU32 index);
static void         vertex_buffer_update        (VertexBuffer vbuf, void* data, nkU64 bytes, BufferType type);
static void         vertex_buffer_draw          (VertexBuffer vbuf, DrawMode draw_mode, nkU64 vert_count);
// =============================================================================

// Shader ======================================================================
DECLARE_PRIVATE_TYPE(Shader);

static Shader shader_create   (void* data, nkU64 bytes);
static void   shader_destroy  (Shader shader);
static void   shader_bind     (Shader shader);
static void   shader_set_bool (Shader shader, const nkChar* name, nkBool val);
static void   shader_set_int  (Shader shader, const nkChar* name, nkS32  val);
static void   shader_set_float(Shader shader, const nkChar* name, nkF32  val);
static void   shader_set_vec2 (Shader shader, const nkChar* name, nkVec2 val);
static void   shader_set_vec3 (Shader shader, const nkChar* name, nkVec3 val);
static void   shader_set_vec4 (Shader shader, const nkChar* name, nkVec4 val);
static void   shader_set_mat2 (Shader shader, const nkChar* name, nkMat2 val);
static void   shader_set_mat3 (Shader shader, const nkChar* name, nkMat3 val);
static void   shader_set_mat4 (Shader shader, const nkChar* name, nkMat4 val);
// =============================================================================

// Texture =====================================================================
DECLARE_PRIVATE_TYPE(Texture);

static Texture texture_create    (nkS32 w, nkS32 h, nkS32 bpp, void* data, SamplerFilter filter, SamplerWrap wrap);
static void    texture_destroy   (Texture texture);
static void    texture_bind      (Texture texture, nkS32 unit);
static nkVec2  texture_get_size  (Texture texture);
static nkF32   texture_get_width (Texture texture);
static nkF32   texture_get_height(Texture texture);
// =============================================================================

// RenderTarget ================================================================
DECLARE_PRIVATE_TYPE(RenderTarget);

static RenderTarget render_target_create (nkS32 w, nkS32 h, SamplerFilter filter, SamplerWrap wrap);
static void         render_target_destroy(RenderTarget target);
static void         render_target_resize (RenderTarget target, nkS32 w, nkS32 h);
static void         render_target_bind   (RenderTarget target);
// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
