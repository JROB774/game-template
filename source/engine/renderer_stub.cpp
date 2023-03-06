/*////////////////////////////////////////////////////////////////////////////*/

// Buffer ======================================================================

DEFINE_PRIVATE_TYPE(Buffer)
{
    // Nothing...
};

GLOBAL Buffer create_buffer(const BufferDesc& desc)
{
    return NULL;
}

GLOBAL void free_buffer(Buffer buffer)
{
    // Nothing...
}

GLOBAL void update_buffer(Buffer buffer, void* data, nkU64 bytes)
{
    // Nothing...
}

// =============================================================================

// Shader ======================================================================

DEFINE_PRIVATE_TYPE(Shader)
{
    // Nothing...
};

GLOBAL Shader create_shader(const ShaderDesc& desc)
{
    return NULL;
}

GLOBAL void free_shader(Shader shader)
{
    // Nothing...
}

// =============================================================================

// Sampler =====================================================================

DEFINE_PRIVATE_TYPE(Sampler)
{
    // Nothing...
};

GLOBAL Sampler create_sampler(const SamplerDesc& desc)
{
    return NULL;
}

GLOBAL void free_sampler(Sampler sampler)
{
    // Nothing...
}

// =============================================================================

// Texture =====================================================================

DEFINE_PRIVATE_TYPE(Texture)
{
    // Nothing...
};

GLOBAL Texture create_texture(const TextureDesc& desc)
{
    return NULL;
}

GLOBAL void free_texture(Texture texture)
{
    // Nothing...
}

GLOBAL void resize_texture(Texture texture, nkS32 width, nkS32 height)
{
    // Nothing...
}

GLOBAL iPoint get_texture_size(Texture texture)
{
    return { 0,0 };
}

GLOBAL nkS32 get_texture_width(Texture texture)
{
    return 0;
}

GLOBAL nkS32 get_texture_height(Texture texture)
{
    return 0;
}

// =============================================================================

// Render Pass =================================================================

DEFINE_PRIVATE_TYPE(RenderPass)
{
    // Nothing...
};

GLOBAL RenderPass create_render_pass(const RenderPassDesc& desc)
{
    return NULL;
}

GLOBAL void free_render_pass(RenderPass pass)
{
    // Nothing...
}

GLOBAL void begin_render_pass(RenderPass pass)
{
    // Nothing...
}

GLOBAL void end_render_pass(void)
{
    // Nothing...
}

// =============================================================================

// Render Pipeline =============================================================

DEFINE_PRIVATE_TYPE(RenderPipeline)
{
    // Nothing...
};

GLOBAL RenderPipeline create_render_pipeline(const RenderPipelineDesc& desc)
{
    return NULL;
}

GLOBAL void free_render_pipeline(RenderPipeline pipeline)
{
    // Nothing...
}

// =============================================================================

// Renderer ====================================================================

GLOBAL void setup_renderer_platform(void)
{
    // Nothing...
}

GLOBAL void init_render_system(void)
{
    // Nothing...
}

GLOBAL void quit_render_system(void)
{
    // Nothing...
}

GLOBAL void maybe_resize_backbuffer(void)
{
    // Nothing...
}

GLOBAL void present_renderer(void)
{
    // Nothing...
}

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    // Nothing...
}

GLOBAL void begin_scissor(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    // Nothing...
}

GLOBAL void end_scissor(void)
{
    // Nothing...
}

GLOBAL void bind_pipeline(RenderPipeline pipeline)
{
    // Nothing...
}

GLOBAL void bind_buffer(Buffer buffer, nkS32 slot)
{
    // Nothing...
}

GLOBAL void bind_texture(Texture texture, Sampler sampler, nkS32 unit)
{
    // Nothing...
}

GLOBAL void draw_arrays(nkU64 vertex_count)
{
    // Nothing...
}

GLOBAL void draw_elements(nkU64 element_count, ElementType element_type, nkU64 byteOffset)
{
    // Nothing...
}

// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
