/*////////////////////////////////////////////////////////////////////////////*/

#include <SDL_syswm.h> // For getting the HWND from the SDL window.

#include <d3d11.h>
#include <d3dcompiler.h>

struct Direct3DBackbuffer
{
    nkS32                   width;
    nkS32                   height;
    ID3D11RenderTargetView* color_target;
    ID3D11Texture2D*        color_texture;
};

struct Direct3DContext
{
    ID3D11Device*           device;
    ID3D11DeviceContext*    device_context;
    IDXGISwapChain*         swap_chain;
    Direct3DBackbuffer      backbuffer;
    RenderPipeline          current_pipeline;
    Buffer                  current_element_buffer;
    nkBool                  pass_started;
};

INTERNAL Direct3DContext g_d3d;

// Buffer ======================================================================

INTERNAL constexpr D3D11_BIND_FLAG BUFFER_TYPE_TO_D3D[] =
{
    D3D11_BIND_VERTEX_BUFFER,
    D3D11_BIND_INDEX_BUFFER,
    D3D11_BIND_CONSTANT_BUFFER
};

INTERNAL constexpr D3D11_USAGE BUFFER_USAGE_TO_D3D[] =
{
    D3D11_USAGE_IMMUTABLE,
    D3D11_USAGE_DYNAMIC,
    D3D11_USAGE_DYNAMIC
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(BUFFER_TYPE_TO_D3D) == BufferType_TOTAL, buffer_type_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(BUFFER_USAGE_TO_D3D) == BufferUsage_TOTAL, buffer_usage_size_mismatch);

DEFINE_PRIVATE_TYPE(Buffer)
{
    ID3D11Buffer* buffer;
    BufferDesc    desc;
    nkU64         max_bytes;
};

GLOBAL void create_buffer_internally(Buffer buffer, const BufferDesc& desc)
{
    if(!buffer) return;

    // If the buffer object already exists then destroy it first.
    if(buffer->buffer)
    {
        buffer->buffer->Release();
        buffer->buffer = NULL;
    }

    D3D11_BUFFER_DESC buffer_desc = NK_ZERO_MEM;
    buffer_desc.ByteWidth      = NK_CAST(UINT, desc.bytes);
    buffer_desc.Usage          = BUFFER_USAGE_TO_D3D[desc.usage];
    buffer_desc.BindFlags      = BUFFER_TYPE_TO_D3D[desc.type];
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT res;

    if(desc.data)
    {
        D3D11_SUBRESOURCE_DATA data = NK_ZERO_MEM;
        data.pSysMem = desc.data;

        res = g_d3d.device->CreateBuffer(&buffer_desc, &data, &buffer->buffer);
    }
    else
    {
        res = g_d3d.device->CreateBuffer(&buffer_desc, NULL, &buffer->buffer);
    }

    if(!SUCCEEDED(res))
    {
        fatal_error("Failed to create Direct3D buffer!");
    }

    buffer->desc = desc;
    buffer->max_bytes = desc.bytes;
}

GLOBAL Buffer create_buffer(const BufferDesc& desc)
{
    Buffer buffer = ALLOCATE_PRIVATE_TYPE(Buffer);
    if(!buffer) fatal_error("Failed to allocate buffer!");

    create_buffer_internally(buffer, desc);

    return buffer;
}

GLOBAL void free_buffer(Buffer buffer)
{
    if(!buffer) return;
    if(buffer->buffer)
        buffer->buffer->Release();
    NK_FREE(buffer);
}

GLOBAL void update_buffer(Buffer buffer, void* data, nkU64 bytes)
{
    if(!buffer) return;

    buffer->desc.data = data;
    buffer->desc.bytes = bytes;

    // Direct3D buffers cannot grow past their initial creation size. If the
    // caller is trying to fill a buffer with more data than it can handle then
    // we re-create it with enough space to handle the data size. Maybe in the
    // future we want to make dynamic buffers a higher-level concept?
    //                                                 -Josh, 10th March 2023
    if(buffer->max_bytes < bytes)
    {
        create_buffer_internally(buffer, buffer->desc);
    }
    else
    {
        D3D11_MAPPED_SUBRESOURCE mapped_data = NK_ZERO_MEM;
        g_d3d.device_context->Map(buffer->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        memcpy(mapped_data.pData, data, bytes);
        g_d3d.device_context->Unmap(buffer->buffer, 0);
    }
}

// =============================================================================

// Shader ======================================================================

DEFINE_PRIVATE_TYPE(Shader)
{
    ID3D11VertexShader* vert_shader;
    ID3D11PixelShader*  frag_shader;
    ID3DBlob*           vert_blob;
    ID3DBlob*           frag_blob;
    UniformDesc         uniforms[32];
    nkU64               uniform_count;
};

GLOBAL Shader create_shader(const ShaderDesc& desc)
{
    Shader shader = ALLOCATE_PRIVATE_TYPE(Shader);
    if(!shader) fatal_error("Failed to allocate shader!");

    ID3DBlob* vs_error_blob = NULL;
    ID3DBlob* fs_error_blob = NULL;

    D3DCompile(desc.data, desc.bytes, NULL, NULL, NULL, "vs_main", "vs_5_0", 0, 0, &shader->vert_blob, &vs_error_blob);
    D3DCompile(desc.data, desc.bytes, NULL, NULL, NULL, "ps_main", "ps_5_0", 0, 0, &shader->frag_blob, &fs_error_blob);

    if(vs_error_blob)
    {
        fatal_error("Vertex shader compilation error:\n\n%.*s\n",
            NK_CAST(nkS32,vs_error_blob->GetBufferSize()), NK_CAST(nkChar*,vs_error_blob->GetBufferPointer()));
    }
    if(fs_error_blob)
    {
        fatal_error("Fragment shader compilation error:\n\n%.*s\n",
            NK_CAST(nkS32,fs_error_blob->GetBufferSize()), NK_CAST(nkChar*,fs_error_blob->GetBufferPointer()));
    }

    HRESULT res;

    res = g_d3d.device->CreateVertexShader(shader->vert_blob->GetBufferPointer(), shader->vert_blob->GetBufferSize(), NULL, &shader->vert_shader);
    if(!SUCCEEDED(res)) fatal_error("Failed to create vertex shader!");
    res = g_d3d.device->CreatePixelShader(shader->frag_blob->GetBufferPointer(), shader->frag_blob->GetBufferSize(), NULL, &shader->frag_shader);
    if(!SUCCEEDED(res)) fatal_error("Failed to create fragment shader!");

    for(nkU64 i=0; i<desc.uniform_count; ++i)
        shader->uniforms[i] = desc.uniforms[i];
    shader->uniform_count = desc.uniform_count;

    return shader;
}

GLOBAL void free_shader(Shader shader)
{
    if(!shader) return;

    if(shader->vert_shader) shader->vert_shader->Release();
    if(shader->vert_blob) shader->vert_blob->Release();
    if(shader->frag_shader) shader->frag_shader->Release();
    if(shader->frag_blob) shader->frag_blob->Release();

    NK_FREE(shader);
}

// =============================================================================

// Sampler =====================================================================

INTERNAL constexpr D3D11_FILTER SAMPLER_FILTER_TO_D3D[] =
{
    D3D11_FILTER_MIN_MAG_MIP_POINT,
    D3D11_FILTER_MIN_MAG_MIP_LINEAR
};

INTERNAL constexpr D3D11_TEXTURE_ADDRESS_MODE SAMPLER_WRAP_TO_D3D[] =
{
    D3D11_TEXTURE_ADDRESS_WRAP,
    D3D11_TEXTURE_ADDRESS_CLAMP
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(SAMPLER_FILTER_TO_D3D) == SamplerFilter_TOTAL, sampler_filter_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(SAMPLER_WRAP_TO_D3D) == SamplerWrap_TOTAL, sampler_wrap_size_mismatch);

DEFINE_PRIVATE_TYPE(Sampler)
{
    ID3D11SamplerState* sampler;
};

GLOBAL Sampler create_sampler(const SamplerDesc& desc)
{
    Sampler sampler = ALLOCATE_PRIVATE_TYPE(Sampler);
    if(!sampler) fatal_error("Failed to allocate sampler!");

    D3D11_SAMPLER_DESC sampler_desc = NK_ZERO_MEM;
    sampler_desc.Filter         = SAMPLER_FILTER_TO_D3D[desc.filter];
    sampler_desc.AddressU       = SAMPLER_WRAP_TO_D3D[desc.wrap_x];
    sampler_desc.AddressV       = SAMPLER_WRAP_TO_D3D[desc.wrap_y];
    sampler_desc.AddressW       = SAMPLER_WRAP_TO_D3D[desc.wrap_z];
    sampler_desc.MipLODBias     = 0;
    sampler_desc.MaxAnisotropy  = 1;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampler_desc.MinLOD         = 0;
    sampler_desc.MaxLOD         = 0;

    HRESULT res = g_d3d.device->CreateSamplerState(&sampler_desc, &sampler->sampler);
    if(!SUCCEEDED(res)) fatal_error("Failed to create sampler state!");

    return sampler;
}

GLOBAL void free_sampler(Sampler sampler)
{
    if(!sampler) return;
    if(sampler->sampler)
        sampler->sampler->Release();
    NK_FREE(sampler);
}

// =============================================================================

// Texture =====================================================================

INTERNAL constexpr DXGI_FORMAT TEXTURE_FORMAT_TO_D3D[] =
{
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_D24_UNORM_S8_UINT
};

INTERNAL constexpr UINT TEXTURE_FORMAT_TO_BPP[] =
{
    1, // DXGI_FORMAT_R8_UNORM
    4, // DXGI_FORMAT_R8G8B8A8_UNORM
    4  // DXGI_FORMAT_D24_UNORM_S8_UINT
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(TEXTURE_FORMAT_TO_D3D) == TextureFormat_TOTAL, texture_format_size_mismatch);

DEFINE_PRIVATE_TYPE(Texture)
{
    ID3D11Texture2D*          texture;
    ID3D11ShaderResourceView* shader_view;
    ID3D11RenderTargetView*   render_target_view;
    ID3D11DepthStencilView*   depth_stencil_view;
    DXGI_FORMAT               format;
    nkS32                     width;
    nkS32                     height;
};

GLOBAL Texture create_texture(const TextureDesc& desc)
{
    Texture texture = ALLOCATE_PRIVATE_TYPE(Texture);
    if(!texture) fatal_error("Failed to allocate texture!");

    texture->format = TEXTURE_FORMAT_TO_D3D[desc.format];

    HRESULT res;

    D3D11_TEXTURE2D_DESC texture_desc = NK_ZERO_MEM;
    texture_desc.Width              = desc.width;
    texture_desc.Height             = desc.height;
    texture_desc.MipLevels          = 1;
    texture_desc.ArraySize          = 1;
    texture_desc.Format             = texture->format;
    texture_desc.SampleDesc.Count   = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Usage              = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    texture_desc.CPUAccessFlags     = 0;
    texture_desc.MiscFlags          = 0;

    if(texture->format == DXGI_FORMAT_D24_UNORM_S8_UINT)
    {
        NK_SET_FLAGS(texture_desc.BindFlags, D3D11_BIND_DEPTH_STENCIL);
    }
    else
    {
        NK_SET_FLAGS(texture_desc.BindFlags, D3D11_BIND_RENDER_TARGET);
    }

    if(desc.data)
    {
        D3D11_SUBRESOURCE_DATA resource = NK_ZERO_MEM;
        resource.pSysMem     = desc.data;
        resource.SysMemPitch = desc.width * TEXTURE_FORMAT_TO_BPP[desc.format];

        res = g_d3d.device->CreateTexture2D(&texture_desc, &resource, &texture->texture);
        if(!SUCCEEDED(res)) fatal_error("Failed to create 2D texture!");
    }
    else
    {
        res = g_d3d.device->CreateTexture2D(&texture_desc, NULL, &texture->texture);
        if(!SUCCEEDED(res)) fatal_error("Failed to create 2D texture!");
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC shader_view_desc = NK_ZERO_MEM;
    shader_view_desc.Format                    = texture->format;
    shader_view_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    shader_view_desc.Texture2D.MostDetailedMip = 0;
    shader_view_desc.Texture2D.MipLevels       = 1;

    res = g_d3d.device->CreateShaderResourceView(texture->texture, &shader_view_desc, &texture->shader_view);
    if(!SUCCEEDED(res)) fatal_error("Failed to create shader view for texture!");

    D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = NK_ZERO_MEM;
    render_target_view_desc.Format             = texture->format;
    render_target_view_desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
    render_target_view_desc.Texture2D.MipSlice = 0;

    res = g_d3d.device->CreateRenderTargetView(texture->texture, &render_target_view_desc, &texture->render_target_view);
    if(!SUCCEEDED(res)) fatal_error("Failed to create render target view for texture!");

    if(texture->format == DXGI_FORMAT_D24_UNORM_S8_UINT)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = NK_ZERO_MEM;
        depth_stencil_view_desc.Format             = texture->format;
        depth_stencil_view_desc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
        depth_stencil_view_desc.Texture2D.MipSlice = 0;

        res = g_d3d.device->CreateDepthStencilView(texture->texture, &depth_stencil_view_desc, &texture->depth_stencil_view);
        if(!SUCCEEDED(res)) fatal_error("Failed to create depth stencil view for texture!");
    }

    texture->width = desc.width;
    texture->height = desc.height;

    return texture;
}

GLOBAL void free_texture(Texture texture)
{
    if(!texture) return;

    if(texture->texture) texture->texture->Release();

    if(texture->render_target_view) texture->render_target_view->Release();
    if(texture->shader_view) texture->shader_view->Release();
    if(texture->depth_stencil_view) texture->depth_stencil_view->Release();

    NK_FREE(texture);
}

GLOBAL void resize_texture(Texture texture, nkS32 width, nkS32 height)
{
    // @Incomplete: ...
}

GLOBAL iPoint get_texture_size(Texture texture)
{
    NK_ASSERT(texture);
    iPoint size;
    size.x = texture->width;
    size.y = texture->height;
    return size;
}

GLOBAL nkS32 get_texture_width(Texture texture)
{
    NK_ASSERT(texture);
    return texture->width;
}

GLOBAL nkS32 get_texture_height(Texture texture)
{
    NK_ASSERT(texture);
    return texture->height;
}

// =============================================================================

// Render Pass =================================================================

DEFINE_PRIVATE_TYPE(RenderPass)
{
    RenderPassDesc desc;
};

GLOBAL RenderPass create_render_pass(const RenderPassDesc& desc)
{
    RenderPass pass = ALLOCATE_PRIVATE_TYPE(RenderPass);
    if(!pass) fatal_error("Failed to allocate render pass!");

    pass->desc = desc;

    return pass;
}

GLOBAL void free_render_pass(RenderPass pass)
{
    if(!pass) return;
    NK_FREE(pass);
}

GLOBAL void begin_render_pass(RenderPass pass)
{
    NK_ASSERT(!g_d3d.pass_started); // Render pass is already in progress!

    g_d3d.pass_started = NK_TRUE;

    ID3D11RenderTargetView* color_views[16] = NK_ZERO_MEM; // Should match the number of targets in RenderPassDesc!
    NK_STATIC_ASSERT(NK_ARRAY_SIZE(color_views) == NK_ARRAY_SIZE(pass->desc.color_targets), view_target_size_mismatch);

    // Clear the target(s).
    for(nkU32 i=0; i<pass->desc.num_color_targets; ++i)
    {
        color_views[i] = (pass->desc.color_targets[i] == BACKBUFFER) ? g_d3d.backbuffer.color_target : pass->desc.color_targets[i]->render_target_view;
        if(pass->desc.clear)
        {
            g_d3d.device_context->ClearRenderTargetView(color_views[i], pass->desc.clear_color.raw);
        }
    }

    ID3D11DepthStencilView* depth_stencil_view = NULL;
    if(pass->desc.depth_stencil_target)
    {
        depth_stencil_view = pass->desc.depth_stencil_target->depth_stencil_view;
        if(pass->desc.clear)
        {
            g_d3d.device_context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 0.0f, 0); // @Todo: Add depth and stencil clear values!
        }
    }

    g_d3d.device_context->OMSetRenderTargets(pass->desc.num_color_targets, color_views, depth_stencil_view);
}

GLOBAL void end_render_pass(void)
{
    NK_ASSERT(g_d3d.pass_started); // Render pass has not been started!
    g_d3d.pass_started = NK_FALSE;
}

// =============================================================================

// Render Pipeline =============================================================

INTERNAL constexpr DXGI_FORMAT ATTRIB_TYPE_TO_D3D[] =
{
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_FLOAT
};

INTERNAL constexpr D3D11_CULL_MODE CULL_FACE_TO_D3D[] =
{
    D3D11_CULL_NONE,
    D3D11_CULL_FRONT,
    D3D11_CULL_BACK
};

INTERNAL constexpr D3D11_COMPARISON_FUNC DEPTH_OP_TO_D3D[] =
{
    D3D11_COMPARISON_NEVER,
    D3D11_COMPARISON_EQUAL,
    D3D11_COMPARISON_NOT_EQUAL,
    D3D11_COMPARISON_LESS,
    D3D11_COMPARISON_LESS_EQUAL,
    D3D11_COMPARISON_GREATER,
    D3D11_COMPARISON_GREATER_EQUAL,
    D3D11_COMPARISON_ALWAYS
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(ATTRIB_TYPE_TO_D3D) == AttribType_TOTAL, attrib_type_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(CULL_FACE_TO_D3D) == CullFace_TOTAL, cull_face_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(DEPTH_OP_TO_D3D) == DepthOp_TOTAL, depth_op_size_mismatch);

DEFINE_PRIVATE_TYPE(RenderPipeline)
{
    RenderPipelineDesc       desc;
    nkU64                    vertex_byte_stride;
    ID3D11InputLayout*       input_layout;
    ID3D11RasterizerState*   rasterizer_state;
    ID3D11BlendState*        blend_state;
    ID3D11DepthStencilState* depth_stencil_state;
};

GLOBAL RenderPipeline create_render_pipeline(const RenderPipelineDesc& desc)
{
    RenderPipeline pipeline = ALLOCATE_PRIVATE_TYPE(RenderPipeline);
    if(!pipeline) fatal_error("Failed to allocate render pipeline!");

    pipeline->desc = desc;

    HRESULT res;

    // Create the input vertex layout.
    pipeline->vertex_byte_stride = desc.vertex_layout.byte_stride;

    D3D11_INPUT_ELEMENT_DESC input_desc[16] = NK_ZERO_MEM; // NOTE: This size should match what is specified in the VertexLayout struct.
    UINT input_count = 0;
    for(nkU64 i=0, n=NK_ARRAY_SIZE(desc.vertex_layout.attribs); i<n; ++i)
    {
        const VertexAttrib* attrib = &desc.vertex_layout.attribs[i];
        if(attrib->enabled)
        {
            D3D11_INPUT_ELEMENT_DESC* input_elem_desc = &input_desc[input_count++];
            input_elem_desc->SemanticName      = attrib->semantic_name;
            input_elem_desc->SemanticIndex     = attrib->semantic_index;
            input_elem_desc->Format            = ATTRIB_TYPE_TO_D3D[attrib->type];
            input_elem_desc->InputSlot         = 0; // @Todo: For the time being we expect vertex buffers in slot 0.
            input_elem_desc->AlignedByteOffset = NK_CAST(UINT,attrib->byte_offset);
            input_elem_desc->InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
        }
    }

    auto vert_blob_data = desc.shader->vert_blob->GetBufferPointer();
    auto vert_blob_size = desc.shader->vert_blob->GetBufferSize();

    res = g_d3d.device->CreateInputLayout(input_desc, input_count, vert_blob_data, vert_blob_size, &pipeline->input_layout);
    if(!SUCCEEDED(res)) fatal_error("Failed to create pipeline input vertex layout!");

    // Create the rasterizer state.
    D3D11_RASTERIZER_DESC rasterizer_desc = NK_ZERO_MEM;
    rasterizer_desc.FillMode              = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode              = CULL_FACE_TO_D3D[desc.cull_face];
    rasterizer_desc.FrontCounterClockwise = NK_TRUE;
    rasterizer_desc.DepthBias             = 0;
    rasterizer_desc.DepthBiasClamp        = 0.0f;
    rasterizer_desc.SlopeScaledDepthBias  = 0.0f;
    rasterizer_desc.DepthClipEnable       = NK_FALSE;
    rasterizer_desc.ScissorEnable         = NK_FALSE; // @Incomplete!
    rasterizer_desc.MultisampleEnable     = NK_FALSE;
    rasterizer_desc.AntialiasedLineEnable = NK_FALSE;

    res = g_d3d.device->CreateRasterizerState(&rasterizer_desc, &pipeline->rasterizer_state);
    if(!SUCCEEDED(res)) fatal_error("Failed to create pipeline rasterizer state!");

    // Create the blend state.
    D3D11_BLEND_DESC blend_desc       = NK_ZERO_MEM;
    blend_desc.AlphaToCoverageEnable  = NK_FALSE;
    blend_desc.IndependentBlendEnable = NK_FALSE;

    switch(desc.blend_mode)
    {
        case BlendMode_None:
        {
            blend_desc.RenderTarget[0].BlendEnable = NK_FALSE;
        } break;
        case BlendMode_Alpha:
        {
            blend_desc.RenderTarget[0].BlendEnable           = NK_TRUE;
            blend_desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
            blend_desc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
            blend_desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
            blend_desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ONE;
            blend_desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        } break;
        case BlendMode_PremultipliedAlpha:
        {
            blend_desc.RenderTarget[0].BlendEnable           = NK_TRUE;
            blend_desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
            blend_desc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
            blend_desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
            blend_desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ONE;
            blend_desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        } break;
    }

    res = g_d3d.device->CreateBlendState(&blend_desc, &pipeline->blend_state);
    if(!SUCCEEDED(res)) fatal_error("Failed to create pipeline blend state!");

    // Create the depth stencil state.
    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = NK_ZERO_MEM;
    depth_stencil_desc.DepthEnable              = desc.depth_read;
    depth_stencil_desc.DepthWriteMask           = (desc.depth_write) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depth_stencil_desc.DepthFunc                = DEPTH_OP_TO_D3D[desc.depth_op];

    res = g_d3d.device->CreateDepthStencilState(&depth_stencil_desc, &pipeline->depth_stencil_state);
    if(!SUCCEEDED(res)) fatal_error("Failed to create pipeline depth stencil state!");

    return pipeline;
}

GLOBAL void free_render_pipeline(RenderPipeline pipeline)
{
    if(!pipeline) return;

    if(pipeline->input_layout) pipeline->input_layout->Release();
    if(pipeline->rasterizer_state) pipeline->rasterizer_state->Release();
    if(pipeline->blend_state) pipeline->blend_state->Release();
    if(pipeline->depth_stencil_state) pipeline->depth_stencil_state->Release();

    NK_FREE(pipeline);
}

// =============================================================================

// Renderer ====================================================================

INTERNAL constexpr D3D11_PRIMITIVE_TOPOLOGY DRAW_MODE_TO_D3D[] =
{
    D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
    D3D_PRIMITIVE_TOPOLOGY_LINELIST,
    D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
};

INTERNAL constexpr DXGI_FORMAT ELEMENT_TYPE_TO_D3D[] =
{
    DXGI_FORMAT_R16_UINT,
    DXGI_FORMAT_R32_UINT
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(DRAW_MODE_TO_D3D) == DrawMode_TOTAL, draw_mode_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(ELEMENT_TYPE_TO_D3D) == ElementType_TOTAL, element_type_size_mismatch);

GLOBAL void setup_renderer_platform(void)
{
    // Doesn't need to do anything in this backend...
}

GLOBAL void init_render_system(void)
{
    printf("[Direct3D]: Initializing System\n");

    HWND hwnd = NULL;

    // Extract the HWND from the SDL window so we can use it for setting up D3D.
    SDL_SysWMinfo win_info = NK_ZERO_MEM;
    SDL_VERSION(&win_info.version);
    if(SDL_GetWindowWMInfo(NK_CAST(SDL_Window*, get_window()), &win_info))
        hwnd = win_info.info.win.window;;
    if(!hwnd)
    {
        fatal_error("Could not get SDL window internal handle!");
    }

    // Setup our D3D11 rendering context.
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };

    nkU32 device_flags = 0;
    #if defined(BUILD_DEBUG)
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif // BUILD_DEBUG

    HRESULT res;

    res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, device_flags, feature_levels,
        NK_ARRAY_SIZE(feature_levels), D3D11_SDK_VERSION, &g_d3d.device, NULL, &g_d3d.device_context);
    if(!SUCCEEDED(res))
    {
        fatal_error("Failed to create Direct3D context!");
    }

    IDXGIDevice*  dxgi_device  = NULL;
    IDXGIAdapter* dxgi_adapter = NULL;
    IDXGIFactory* dxgi_factory = NULL;

    g_d3d.device->QueryInterface(__uuidof(IDXGIDevice ), NK_CAST(void**,&dxgi_device ));
    dxgi_device->GetParent(      __uuidof(IDXGIAdapter), NK_CAST(void**,&dxgi_adapter));
    dxgi_adapter->GetParent(     __uuidof(IDXGIFactory), NK_CAST(void**,&dxgi_factory));

    // Setup our swap chain.
    DXGI_SWAP_CHAIN_DESC swap_chain_desc               = NK_ZERO_MEM;
    swap_chain_desc.BufferDesc.Width                   = get_window_width();
    swap_chain_desc.BufferDesc.Height                  = get_window_height();
    swap_chain_desc.BufferDesc.RefreshRate.Numerator   = NK_CAST(nkU32,get_app_desc()->tick_rate); // Our preferred refresh rate without VSync.
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count                   = 1; // We don't care about multi-sampling.
    swap_chain_desc.SampleDesc.Quality                 = 0;
    swap_chain_desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount                        = 2; // Double-buffered rendering.
    swap_chain_desc.OutputWindow                       = hwnd;
    swap_chain_desc.Windowed                           = NK_TRUE;
    swap_chain_desc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    res = dxgi_factory->CreateSwapChain(g_d3d.device, &swap_chain_desc, &g_d3d.swap_chain);
    if(!SUCCEEDED(res))
    {
        fatal_error("Failed to create Direct3D swap chain!");
    }

    // Setup our main backbuffer render target.
    g_d3d.swap_chain->GetBuffer(0, IID_PPV_ARGS(&g_d3d.backbuffer.color_texture));
    res = g_d3d.device->CreateRenderTargetView(g_d3d.backbuffer.color_texture, NULL, &g_d3d.backbuffer.color_target);
    if(!SUCCEEDED(res))
        fatal_error("Failed to create Direct3D backbuffer!");
    g_d3d.backbuffer.width = get_window_width();
    g_d3d.backbuffer.height = get_window_height();

    // Disable DXGI intercepting window events as it is annoying.
    dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES);
}

GLOBAL void quit_render_system(void)
{
    if(g_d3d.backbuffer.color_texture)
    {
        g_d3d.backbuffer.color_texture->Release();
        g_d3d.backbuffer.color_texture = NULL;
    }
    if(g_d3d.backbuffer.color_target)
    {
        g_d3d.backbuffer.color_target->Release();
        g_d3d.backbuffer.color_target = NULL;
    }
    if(g_d3d.swap_chain)
    {
        g_d3d.swap_chain->Release();
        g_d3d.swap_chain = NULL;
    }
    if(g_d3d.device_context)
    {
        g_d3d.device_context->Release();
        g_d3d.device_context = NULL;
    }
    if(g_d3d.device)
    {
        g_d3d.device->Release();
        g_d3d.device = NULL;
    }
}

GLOBAL void maybe_resize_backbuffer(void)
{
    // We need to resize the backbuffer if the window has changed size.
    nkS32 ww = get_window_width();
    nkS32 wh = get_window_height();

    if(ww != g_d3d.backbuffer.width || wh != g_d3d.backbuffer.height)
    {
        g_d3d.device_context->OMSetRenderTargets(0, NULL, NULL);

        if(g_d3d.backbuffer.color_texture)
        {
            g_d3d.backbuffer.color_texture->Release();
            g_d3d.backbuffer.color_texture = NULL;
            g_d3d.backbuffer.color_target->Release();
            g_d3d.backbuffer.color_target = NULL;
        }

        g_d3d.swap_chain->ResizeBuffers(0, ww, wh, DXGI_FORMAT_UNKNOWN, 0);

        g_d3d.swap_chain->GetBuffer(0, IID_PPV_ARGS(&g_d3d.backbuffer.color_texture));
        HRESULT res = g_d3d.device->CreateRenderTargetView(g_d3d.backbuffer.color_texture, NULL, &g_d3d.backbuffer.color_target);
        if(!SUCCEEDED(res))
            fatal_error("Failed to resize Direct3D backbuffer!");
        g_d3d.backbuffer.width = ww;
        g_d3d.backbuffer.height = wh;
    }
}

GLOBAL void present_renderer(void)
{
    g_d3d.swap_chain->Present(1, 0); // Present with Vsync.
}

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    D3D11_VIEWPORT viewport = NK_ZERO_MEM;
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.Width    = w;
    viewport.Height   = h;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    g_d3d.device_context->RSSetViewports(1, &viewport);
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
    NK_ASSERT(g_d3d.pass_started); // Cannot bind outside of a render pass!

    if(!pipeline) return;

    g_d3d.current_pipeline = pipeline;

    g_d3d.device_context->IASetPrimitiveTopology(DRAW_MODE_TO_D3D[pipeline->desc.draw_mode]);
    g_d3d.device_context->IASetInputLayout(pipeline->input_layout);
    g_d3d.device_context->RSSetState(pipeline->rasterizer_state);
    g_d3d.device_context->VSSetShader(pipeline->desc.shader->vert_shader, NULL, 0);
    g_d3d.device_context->PSSetShader(pipeline->desc.shader->frag_shader, NULL, 0);
    g_d3d.device_context->OMSetBlendState(pipeline->blend_state, NULL, 0xFFFFFFFF);
    g_d3d.device_context->OMSetDepthStencilState(pipeline->depth_stencil_state, 0);
}

GLOBAL void bind_buffer(Buffer buffer, nkS32 slot)
{
    NK_ASSERT(g_d3d.pass_started); // Cannot bind outside of a render pass!

    if(!buffer) return;

    switch(buffer->desc.type)
    {
        case BufferType_Vertex:
        {
            UINT byte_stride = NK_CAST(UINT,g_d3d.current_pipeline->vertex_byte_stride);
            UINT offset = 0;
            g_d3d.device_context->IASetVertexBuffers(0, 1, &buffer->buffer, &byte_stride, &offset);
        } break;
        case BufferType_Element:
        {
            g_d3d.current_element_buffer = buffer; // Element buffers our bound in the draw call functions.
        } break;
        case BufferType_Uniform:
        {
            g_d3d.device_context->VSSetConstantBuffers(slot, 1, &buffer->buffer);
            g_d3d.device_context->PSSetConstantBuffers(slot, 1, &buffer->buffer);
        } break;
    }
}

GLOBAL void bind_texture(Texture texture, Sampler sampler, nkS32 unit)
{
    NK_ASSERT(g_d3d.pass_started); // Cannot bind outside of a render pass!

    if(!texture) return;
    if(!sampler) return;

    g_d3d.device_context->VSSetShaderResources(unit, 1, &texture->shader_view);
    g_d3d.device_context->VSSetSamplers(unit, 1, &sampler->sampler);

    g_d3d.device_context->PSSetShaderResources(unit, 1, &texture->shader_view);
    g_d3d.device_context->PSSetSamplers(unit, 1, &sampler->sampler);
}

GLOBAL void draw_arrays(nkU64 vertex_count)
{
    NK_ASSERT(g_d3d.pass_started); // Cannot draw outside of a render pass!

    if(vertex_count == 0) return;

    g_d3d.device_context->Draw(NK_CAST(UINT,vertex_count), 0);
}

GLOBAL void draw_elements(nkU64 element_count, ElementType element_type, nkU64 byte_offset)
{
    NK_ASSERT(g_d3d.pass_started); // Cannot draw outside of a render pass!
    NK_ASSERT(g_d3d.current_element_buffer); // We need an element buffer bound for indexed drawing.

    if(element_count == 0) return;

    DXGI_FORMAT type = ELEMENT_TYPE_TO_D3D[element_type];

    g_d3d.device_context->IASetIndexBuffer(g_d3d.current_element_buffer->buffer, type, NK_CAST(UINT,byte_offset));
    g_d3d.device_context->DrawIndexed(NK_CAST(UINT,element_count), 0, 0);
}

// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
