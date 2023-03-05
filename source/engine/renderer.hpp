/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void init_render_system     (void);
GLOBAL void quit_render_system     (void);
GLOBAL void setup_renderer_platform(void);

// Buffer ======================================================================
DECLARE_PRIVATE_TYPE(Buffer);

NK_ENUM(BufferType, nkS32)
{
    BufferType_Vertex,
    BufferType_Element,
    BufferType_Uniform,
    BufferType_TOTAL
};

NK_ENUM(BufferUsage, nkS32)
{
    BufferUsage_Static,
    BufferUsage_Dynamic,
    BufferUsage_Stream,
    BufferUsage_TOTAL
};

struct BufferDesc
{
    BufferType  type  = BufferType_Vertex;
    BufferUsage usage = BufferUsage_Static;
    void*       data  = NULL;
    nkU64       bytes = 0;
};

GLOBAL Buffer create_buffer(const BufferDesc& desc);
GLOBAL void   free_buffer  (Buffer buffer);
GLOBAL void   update_buffer(Buffer buffer, void* data, nkU64 bytes);
// =============================================================================

// Shader ======================================================================
DECLARE_PRIVATE_TYPE(Shader);

NK_ENUM(UniformType, nkS32)
{
    UniformType_Buffer,
    UniformType_Texture,
    UniformType_TOTAL
};

struct UniformDesc
{
    nkString    name;
    UniformType type = UniformType_Buffer;
    nkS32       bind = 0;
};

struct ShaderDesc
{
    void*       data          = NULL;
    nkU64       bytes         = 0;
    UniformDesc uniforms[32]  = {};
    nkU64       uniform_count = 0;
};

GLOBAL Shader create_shader(const ShaderDesc& desc);
GLOBAL void   free_shader  (Shader shader);
// =============================================================================

// Sampler =====================================================================
DECLARE_PRIVATE_TYPE(Sampler);

NK_ENUM(SamplerFilter, nkS32)
{
    SamplerFilter_Nearest,
    SamplerFilter_Linear,
    SamplerFilter_TOTAL
};

NK_ENUM(SamplerWrap, nkS32)
{
    SamplerWrap_Repeat,
    SamplerWrap_Clamp,
    SamplerWrap_TOTAL
};

struct SamplerDesc
{
    SamplerFilter filter = SamplerFilter_Nearest;
    SamplerWrap   wrap_x = SamplerWrap_Clamp;
    SamplerWrap   wrap_y = SamplerWrap_Clamp;
    SamplerWrap   wrap_z = SamplerWrap_Clamp;
};

GLOBAL Sampler create_sampler(const SamplerDesc& desc);
GLOBAL void    free_sampler  (Sampler sampler);
// =============================================================================

// Texture =====================================================================
DECLARE_PRIVATE_TYPE(Texture);

NK_ENUM(TextureType, nkS32)
{
    TextureType_2D,
    TextureType_TOTAL
};

NK_ENUM(TextureFormat, nkS32)
{
    TextureFormat_R,
    TextureFormat_RGB,
    TextureFormat_RGBA,
    TextureFormat_D24S8,
    TextureFormat_TOTAL
};

struct TextureDesc
{
    TextureType   type   = TextureType_2D;
    TextureFormat format = TextureFormat_RGBA;
    nkS32         width  = 0;
    nkS32         height = 0;
    void*         data   = NULL;
};

GLOBAL Texture create_texture    (const TextureDesc& desc);
GLOBAL void    free_texture      (Texture texture);
GLOBAL void    resize_texture    (Texture texture, nkS32 width, nkS32 height);
GLOBAL nkVec2  get_texture_size  (Texture texture);
GLOBAL nkS32   get_texture_width (Texture texture);
GLOBAL nkS32   get_texture_height(Texture texture);
// =============================================================================

// Render Pass =================================================================
INTERNAL constexpr Texture BACKBUFFER = NULL;

DECLARE_PRIVATE_TYPE(RenderPass);

NK_ENUM(DrawMode, nkS32)
{
    DrawMode_Points,
    DrawMode_Lines,
    DrawMode_LineStrip,
    DrawMode_Triangles,
    DrawMode_TriangleStrip,
    DrawMode_TOTAL
};

NK_ENUM(BlendMode, nkS32)
{
    BlendMode_None,
    BlendMode_Alpha,
    BlendMode_PremultipliedAlpha,
    BlendMode_TOTAL
};

NK_ENUM(CullFace, nkS32)
{
    CullFace_None,
    CullFace_Front,
    CullFace_Back,
    CullFace_FrontAndBack,
    CullFace_TOTAL
};

NK_ENUM(DepthOp, nkS32)
{
    DepthOp_Never,
    DepthOp_Equal,
    DepthOp_NotEqual,
    DepthOp_Less,
    DepthOp_LessEqual,
    DepthOp_Greater,
    DepthOp_GreaterEqual,
    DepthOp_Always,
    DepthOp_TOTAL
};

NK_ENUM(ElementType, nkS32)
{
    ElementType_UnsignedByte,
    ElementType_UnsignedShort,
    ElementType_UnsignedInt,
    ElementType_TOTAL
};

NK_ENUM(AttribType, nkS32)
{
    AttribType_SignedByte,
    AttribType_UnsignedByte,
    AttribType_SignedInt,
    AttribType_UnsignedInt,
    AttribType_Float,
    AttribType_TOTAL
};

struct VertexAttrib
{
    nkU32      index       = 0;
    AttribType type        = AttribType_Float;
    nkU32      components  = 4;
    nkU64      byte_offset = 0;
    nkBool     enabled     = NK_FALSE;
};

struct VertexLayout
{
    VertexAttrib attribs[16]  = {};
    nkU64        attrib_count = 0;
    nkU64        byte_stride  = 0;
};

struct RenderPassDesc
{
    Texture      color_targets[16]    = { BACKBUFFER };
    Texture      depth_stencil_target = NULL;
    nkU32        num_color_targets    = 1;
    DrawMode     draw_mode            = DrawMode_Triangles;
    BlendMode    blend_mode           = BlendMode_None;
    CullFace     cull_face            = CullFace_Back;
    DepthOp      depth_op             = DepthOp_Less;
    nkBool       depth_read           = NK_TRUE;
    nkBool       depth_write          = NK_TRUE;
    nkBool       clear                = NK_FALSE;
    nkVec4       clear_color          = NK_V4_BLACK;
};

GLOBAL RenderPass create_render_pass(const RenderPassDesc& desc);
GLOBAL void       free_render_pass  (RenderPass pass);
GLOBAL void       begin_render_pass (RenderPass pass);
GLOBAL void       end_render_pass   (void);
GLOBAL void       set_viewport      (nkF32 x, nkF32 y, nkF32 w, nkF32 h);
GLOBAL void       begin_scissor     (nkF32 x, nkF32 y, nkF32 w, nkF32 h);
GLOBAL void       end_scissor       (void);
GLOBAL void       bind_buffer       (Buffer buffer, nkS32 slot = 0);
GLOBAL void       bind_shader       (Shader shader);
GLOBAL void       bind_texture      (Texture texture, Sampler sampler, nkS32 unit);
GLOBAL void       draw_arrays       (const VertexLayout& vertex_layout, nkU64 vertex_count);
GLOBAL void       draw_elements     (const VertexLayout& vertex_layout, nkU64 element_count, ElementType element_type, nkU64 byteOffset = 0);
// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
