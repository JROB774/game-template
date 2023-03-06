/*////////////////////////////////////////////////////////////////////////////*/

#include <SDL_syswm.h> // For getting the HWND from the SDL window.

#include <d3d11.h>
#include <d3dcompiler.h>

GLOBAL void init_render_system(void)
{
    printf("[Direct3D]: Initializing System\n");

    // @Incomplete: ...
}

GLOBAL void quit_render_system(void)
{
    // @Incomplete: ...
}

GLOBAL void setup_renderer_platform(void)
{
    // Does nothing...
}

// Buffer ======================================================================

DEFINE_PRIVATE_TYPE(Buffer)
{
    // @Incomplete: ...
};

GLOBAL Buffer create_buffer(const BufferDesc& desc)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_buffer(Buffer buffer)
{
    // @Incomplete: ...
}

GLOBAL void update_buffer(Buffer buffer, void* data, nkU64 bytes)
{
    // @Incomplete: ...
}

// =============================================================================

// Shader ======================================================================

DEFINE_PRIVATE_TYPE(Shader)
{
    // @Incomplete: ...
};

GLOBAL Shader create_shader(const ShaderDesc& desc)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_shader(Shader shader)
{
    // @Incomplete: ...
}

// =============================================================================

// Sampler =====================================================================

DEFINE_PRIVATE_TYPE(Sampler)
{
    // @Incomplete: ...
};

GLOBAL Sampler create_sampler(const SamplerDesc& desc)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_sampler(Sampler sampler)
{
    // @Incomplete: ...
}

// =============================================================================

// Texture =====================================================================

DEFINE_PRIVATE_TYPE(Texture)
{
    // @Incomplete: ...
};

GLOBAL Texture create_texture(const TextureDesc& desc)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_texture(Texture texture)
{
    // @Incomplete: ...
}

GLOBAL void resize_texture(Texture texture, nkS32 width, nkS32 height)
{
    // @Incomplete: ...
}

GLOBAL nkVec2 get_texture_size(Texture texture)
{
    // @Incomplete: ...
    return NK_V2_ZERO;
}

GLOBAL nkS32 get_texture_width(Texture texture)
{
    // @Incomplete: ...
    return 0;
}

GLOBAL nkS32 get_texture_height(Texture texture)
{
    // @Incomplete: ...
    return 0;
}

// =============================================================================

// Render Pass =================================================================

DEFINE_PRIVATE_TYPE(RenderPass)
{
    // @Incomplete: ...
};

GLOBAL RenderPass create_render_pass(const RenderPassDesc& desc)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_render_pass(RenderPass pass)
{
    // @Incomplete: ...
}

GLOBAL void begin_render_pass(RenderPass pass)
{
    // @Incomplete: ...
}

GLOBAL void end_render_pass(void)
{
    // @Incomplete: ...
}

// =============================================================================

// Render Pipeline =============================================================

DEFINE_PRIVATE_TYPE(RenderPipeline)
{
    // @Incomplete: ...
};

GLOBAL RenderPipeline create_render_pipeline(const RenderPipelineDesc& desc)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_render_pipeline(RenderPipeline pipeline)
{
    // @Incomplete: ...
}

// =============================================================================

// Renderer ====================================================================

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    // @Incomplete: ...
}

GLOBAL void begin_scissor(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    // @Incomplete: ...
}

GLOBAL void end_scissor(void)
{
    // @Incomplete: ...
}

GLOBAL void bind_pipeline(RenderPipeline pipeline)
{
    // @Incomplete: ...
}

GLOBAL void bind_buffer(Buffer buffer, nkS32 slot)
{
    // @Incomplete: ...
}

GLOBAL void bind_texture(Texture texture, Sampler sampler, nkS32 unit)
{
    // @Incomplete: ...
}

GLOBAL void draw_arrays(nkU64 vertex_count)
{
    // @Incomplete: ...
}

GLOBAL void draw_elements(nkU64 element_count, ElementType element_type, nkU64 byteOffset)
{
    // @Incomplete: ...
}

// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
