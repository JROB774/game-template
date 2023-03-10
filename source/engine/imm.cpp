/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkU32 IMM_MAX_UNIFORMS = 8;
INTERNAL constexpr nkU32 IMM_MAX_TEXTURES = 16;

struct ImmVertex
{
    nkVec4 position;
    nkVec4 normal;
    nkVec4 color;
    nkVec4 texcoord;
    nkVec4 userdata0;
    nkVec4 userdata1;
    nkVec4 userdata2;
    nkVec4 userdata3;
};

struct ImmData
{
    void* data;
    nkU64 size;
};

struct ImmUniform
{
    nkMat4 u_projection;
    nkMat4 u_view;
    nkMat4 u_model;
    nkBool u_usetex;
};

struct ImmContext
{
    VertexLayout       vertex_layout;
    nkArray<ImmVertex> vertices;
    Buffer             vertex_buffer;
    Buffer             uniform_buffers[IMM_MAX_UNIFORMS];
    RenderPass         render_pass;
    RenderPipeline     render_pipeline;

    Shader             default_shader;
    Sampler            default_samplers[ImmSampler_TOTAL];

    nkVec4             clear_color = NK_V4_BLACK;
    nkBool             should_clear;

    DrawMode           current_draw_mode;
    Texture            current_color_target;
    Texture            current_depth_target;
    ImmData            current_uniforms[IMM_MAX_UNIFORMS];
    Shader             current_shader;
    Sampler            current_samplers[IMM_MAX_TEXTURES];
    Texture            current_textures[IMM_MAX_TEXTURES];
    fRect              current_viewport;
    nkMat4             current_projection;
    nkMat4             current_view;
    nkMat4             current_model;
    nkBool             current_depth_read;
    nkBool             current_depth_write;

    nkU64              position_count;
    nkU64              normal_count;
    nkU64              color_count;
    nkU64              texcoord_count;
    nkU64              userdata0_count;
    nkU64              userdata1_count;
    nkU64              userdata2_count;
    nkU64              userdata3_count;

    nkBool             draw_started;
    nkBool             pipeline_needs_rebuild;
    nkBool             pass_needs_rebuild;
};

INTERNAL ImmContext g_imm;

GLOBAL void imm_init(void)
{
    g_imm.vertex_layout.attribs[0] = { 0, "POSITION",  0, AttribType_Float4, offsetof(ImmVertex, position ), NK_TRUE };
    g_imm.vertex_layout.attribs[1] = { 1, "NORMAL",    0, AttribType_Float4, offsetof(ImmVertex, normal   ), NK_TRUE };
    g_imm.vertex_layout.attribs[2] = { 2, "COLOR",     0, AttribType_Float4, offsetof(ImmVertex, color    ), NK_TRUE };
    g_imm.vertex_layout.attribs[3] = { 3, "TEXCOORD",  0, AttribType_Float4, offsetof(ImmVertex, texcoord ), NK_TRUE };
    g_imm.vertex_layout.attribs[4] = { 4, "USERDATA",  0, AttribType_Float4, offsetof(ImmVertex, userdata0), NK_TRUE };
    g_imm.vertex_layout.attribs[5] = { 5, "USERDATA",  1, AttribType_Float4, offsetof(ImmVertex, userdata1), NK_TRUE };
    g_imm.vertex_layout.attribs[6] = { 6, "USERDATA",  2, AttribType_Float4, offsetof(ImmVertex, userdata2), NK_TRUE };
    g_imm.vertex_layout.attribs[7] = { 7, "USERDATA",  3, AttribType_Float4, offsetof(ImmVertex, userdata3), NK_TRUE };
    g_imm.vertex_layout.attrib_count = 8;
    g_imm.vertex_layout.byte_stride = sizeof(ImmVertex);

    BufferDesc vbuffer_desc;
    vbuffer_desc.usage = BufferUsage_Dynamic;
    vbuffer_desc.type  = BufferType_Vertex;
    vbuffer_desc.bytes = NK_KB_TO_BYTES(16);
    g_imm.vertex_buffer = create_buffer(vbuffer_desc);

    BufferDesc ubuffer_desc;
    ubuffer_desc.usage = BufferUsage_Dynamic;
    ubuffer_desc.type  = BufferType_Uniform;
    ubuffer_desc.bytes = NK_KB_TO_BYTES(4);
    for(nkS32 i=0; i<IMM_MAX_UNIFORMS; ++i)
    {
        g_imm.uniform_buffers[i] = create_buffer(ubuffer_desc);
    }

    g_imm.default_shader = asset_manager_load<Shader>("imm.shader");

    // Create some common samplers.
    SamplerDesc sd;

    sd.wrap_x = SamplerWrap_Clamp;
    sd.wrap_y = SamplerWrap_Clamp;
    sd.wrap_z = SamplerWrap_Clamp;

    sd.filter = SamplerFilter_Nearest;
    g_imm.default_samplers[ImmSampler_ClampNearest] = create_sampler(sd);
    sd.filter = SamplerFilter_Linear;
    g_imm.default_samplers[ImmSampler_ClampLinear] = create_sampler(sd);

    sd.wrap_x = SamplerWrap_Repeat;
    sd.wrap_y = SamplerWrap_Repeat;
    sd.wrap_z = SamplerWrap_Repeat;

    sd.filter = SamplerFilter_Nearest;
    g_imm.default_samplers[ImmSampler_RepeatNearest] = create_sampler(sd);
    sd.filter = SamplerFilter_Linear;
    g_imm.default_samplers[ImmSampler_RepeatLinear] = create_sampler(sd);
}

GLOBAL void imm_quit(void)
{
    for(nkS32 i=0; i<ImmSampler_TOTAL; ++i)
        free_sampler(g_imm.default_samplers[i]);

    free_buffer(g_imm.vertex_buffer);

    for(nkS32 i=0; i<IMM_MAX_UNIFORMS; ++i)
        free_buffer(g_imm.uniform_buffers[i]);

    if(g_imm.render_pipeline)
        free_render_pipeline(g_imm.render_pipeline);

    if(g_imm.render_pass)
        free_render_pass(g_imm.render_pass);
}

GLOBAL void imm_begin_frame(void)
{
    imm_reset();
}

GLOBAL void imm_end_frame(void)
{
    // Does nothing currently...
}

// General =====================================================================

GLOBAL void imm_clear(nkVec4 color)
{
    // Do an empty render pass where we just clear the current render target.
    g_imm.clear_color = color;
    imm_begin(g_imm.current_draw_mode, NK_TRUE); // Draw mode doesn't matter but we need to define it...
    imm_end();
}

GLOBAL void imm_clear(nkVec3 color)
{
    imm_clear({ color.r,color.g,color.b,1.0f });
}

GLOBAL void imm_clear(nkF32 r, nkF32 g, nkF32 b, nkF32 a)
{
    imm_clear({ r,g,b,a });
}

GLOBAL void imm_reset(void)
{
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    g_imm.current_color_target = NULL;
    g_imm.current_depth_target = NULL;
    g_imm.current_shader       = NULL;
    g_imm.current_viewport     = { 0.0f,0.0f,ww,wh };
    g_imm.current_projection   = nk_orthographic(0,ww,wh,0,-1,1);
    g_imm.current_view         = nk_m4_identity();
    g_imm.current_model        = nk_m4_identity();
    g_imm.current_depth_read   = NK_FALSE;
    g_imm.current_depth_write  = NK_FALSE;

    memset(g_imm.current_uniforms, 0, sizeof(g_imm.current_uniforms));
    memset(g_imm.current_samplers, 0, sizeof(g_imm.current_samplers));
    memset(g_imm.current_textures, 0, sizeof(g_imm.current_textures));

    nk_array_clear(&g_imm.vertices);

    g_imm.pipeline_needs_rebuild = NK_TRUE;
    g_imm.pass_needs_rebuild = NK_TRUE;
}

// =============================================================================

// State Setters ===============================================================

GLOBAL void imm_set_color_target(Texture target)
{
    g_imm.current_color_target = target;
    g_imm.pass_needs_rebuild = NK_TRUE;
}

GLOBAL void imm_set_depth_target(Texture target)
{
    g_imm.current_depth_target = target;
    g_imm.pass_needs_rebuild = NK_TRUE;
}

GLOBAL void imm_set_uniforms(void* data, nkU64 bytes, nkU32 slot)
{
    NK_ASSERT(slot > 0 && slot < IMM_MAX_UNIFORMS); // Slot 0 is off limits (it is used internally).
    g_imm.current_uniforms[slot] = { data, bytes };
}

GLOBAL void imm_set_shader(Shader shader)
{
    NK_ASSERT(!g_imm.draw_started); // Cannot change shader once a draw has started.
    g_imm.current_shader = shader;
    g_imm.pipeline_needs_rebuild = NK_TRUE;
}

GLOBAL void imm_set_sampler(Sampler sampler, nkS32 slot)
{
    NK_ASSERT(slot >= 0 && slot < IMM_MAX_TEXTURES);
    g_imm.current_samplers[slot] = sampler;
}

GLOBAL void imm_set_texture(Texture texture, nkS32 slot)
{
    NK_ASSERT(slot >= 0 && slot < IMM_MAX_TEXTURES);
    g_imm.current_textures[slot] = texture;
}

GLOBAL void imm_set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    g_imm.current_viewport = { x,y,w,h };
}

GLOBAL void imm_set_projection(nkMat4 projection)
{
    g_imm.current_projection = projection;
}

GLOBAL void imm_set_view(nkMat4 view)
{
    g_imm.current_view = view;
}

GLOBAL void imm_set_model(nkMat4 model)
{
    g_imm.current_model = model;
}

GLOBAL void imm_set_depth_read(nkBool enable)
{
    NK_ASSERT(!g_imm.draw_started); // Cannot change depth state once a draw has started.
    g_imm.current_depth_read = enable;
    g_imm.pipeline_needs_rebuild = NK_TRUE;
}

GLOBAL void imm_set_depth_write(nkBool enable)
{
    NK_ASSERT(!g_imm.draw_started); // Cannot change depth state once a draw has started.
    g_imm.current_depth_write = enable;
    g_imm.pipeline_needs_rebuild = NK_TRUE;
}

// =============================================================================

// State Getters ===============================================================

GLOBAL Texture imm_get_color_target(void)
{
    return g_imm.current_color_target;
}

GLOBAL Texture imm_get_depth_target(void)
{
    return g_imm.current_depth_target;
}

GLOBAL Shader imm_get_shader(void)
{
    return g_imm.current_shader;
}

GLOBAL Sampler imm_get_sampler(nkS32 slot)
{
    NK_ASSERT(slot >= 0 && slot < IMM_MAX_TEXTURES);
    return g_imm.current_samplers[slot];
}

GLOBAL Texture imm_get_texture(nkS32 slot)
{
    NK_ASSERT(slot >= 0 && slot < IMM_MAX_TEXTURES);
    return g_imm.current_textures[slot];
}

GLOBAL fRect imm_get_viewport(void)
{
    return g_imm.current_viewport;
}

GLOBAL nkMat4 imm_get_projection(void)
{
    return g_imm.current_projection;
}

GLOBAL nkMat4 imm_get_view(void)
{
    return g_imm.current_view;
}

GLOBAL nkMat4 imm_get_model(void)
{
    return g_imm.current_model;
}

GLOBAL nkBool imm_get_depth_read(void)
{
    return g_imm.current_depth_read;
}

GLOBAL nkBool imm_get_depth_write(void)
{
    return g_imm.current_depth_write;
}

GLOBAL Sampler imm_get_def_sampler(ImmSampler samp)
{
    return g_imm.default_samplers[samp];
}

// =============================================================================

// Polygon Drawing =============================================================

INTERNAL void imm_grow_vertex_array_if_necessary(nkU64 position)
{
    if(g_imm.vertices.length <= position)
        nk_array_append(&g_imm.vertices, ImmVertex());
}

GLOBAL void imm_begin(DrawMode draw_mode, nkBool should_clear)
{
    NK_ASSERT(!g_imm.draw_started); // Cannot start a new draw inside an existing one!

    g_imm.draw_started = NK_TRUE;

    if(g_imm.should_clear != should_clear)
        g_imm.pass_needs_rebuild = NK_TRUE;
    g_imm.should_clear = should_clear;

    if(g_imm.current_draw_mode != draw_mode)
        g_imm.pipeline_needs_rebuild = NK_TRUE;
    g_imm.current_draw_mode = draw_mode;

    // If the pass needs rebuilding then so does the pipeline...
    if(g_imm.pass_needs_rebuild)
    {
        g_imm.pipeline_needs_rebuild = NK_TRUE;
    }

    g_imm.position_count  = 0;
    g_imm.normal_count    = 0;
    g_imm.color_count     = 0;
    g_imm.texcoord_count  = 0;
    g_imm.userdata0_count = 0;
    g_imm.userdata1_count = 0;
    g_imm.userdata2_count = 0;
    g_imm.userdata3_count = 0;

    // Rebuild the render pass if necessary, e.g. when some parameter has changed.
    if(g_imm.pass_needs_rebuild)
    {
        g_imm.pass_needs_rebuild = NK_FALSE;

        if(g_imm.render_pass) free_render_pass(g_imm.render_pass);

        RenderPassDesc pass_desc;
        pass_desc.color_targets[0]     = g_imm.current_color_target;
        pass_desc.depth_stencil_target = g_imm.current_depth_target;
        pass_desc.num_color_targets    = 1;
        pass_desc.clear                = g_imm.should_clear;
        pass_desc.clear_color          = g_imm.clear_color;
        g_imm.render_pass = create_render_pass(pass_desc);
    }

    // Rebuild the render pipeline if necessary, e.g. when some parameter has changed.
    if(g_imm.pipeline_needs_rebuild)
    {
        g_imm.pipeline_needs_rebuild = NK_FALSE;

        if(g_imm.render_pipeline) free_render_pipeline(g_imm.render_pipeline);

        Shader current_shader = ((g_imm.current_shader) ? g_imm.current_shader : g_imm.default_shader);

        RenderPipelineDesc pipeline_desc;
        pipeline_desc.vertex_layout = g_imm.vertex_layout;
        pipeline_desc.render_pass   = g_imm.render_pass;
        pipeline_desc.shader        = current_shader;
        pipeline_desc.draw_mode     = g_imm.current_draw_mode;
        pipeline_desc.blend_mode    = BlendMode_Alpha;
        pipeline_desc.cull_face     = CullFace_None;
        pipeline_desc.depth_read    = g_imm.current_depth_read;
        pipeline_desc.depth_write   = g_imm.current_depth_write;
        g_imm.render_pipeline = create_render_pipeline(pipeline_desc);
    }
}

GLOBAL void imm_end(void)
{
    NK_ASSERT(g_imm.draw_started); // Cannot end a draw that has not been started!

    set_viewport(g_imm.current_viewport.x, g_imm.current_viewport.y, g_imm.current_viewport.w, g_imm.current_viewport.h);

    begin_render_pass(g_imm.render_pass);

    // Setup default samplers.
    for(nkS32 i=0; i<IMM_MAX_TEXTURES; ++i)
    {
        if(!g_imm.current_samplers[i])
            g_imm.current_samplers[i] = g_imm.default_samplers[ImmSampler_ClampNearest];
    }

    // Bind pipeline.
    bind_pipeline(g_imm.render_pipeline);

    // Bind data.
    bind_buffer(g_imm.vertex_buffer);

    // Bind textures.
    nkBool use_texture = NK_FALSE;
    if(g_imm.texcoord_count != 0)
    {
        for(nkS32 i=0; i<IMM_MAX_TEXTURES; ++i)
        {
            if(g_imm.current_textures[i])
            {
                use_texture = NK_TRUE;
                break;
            }
        }
    }
    if(use_texture)
    {
        for(nkS32 i=0; i<IMM_MAX_TEXTURES; ++i)
        {
            if(g_imm.current_textures[i])
                bind_texture(g_imm.current_textures[i], g_imm.current_samplers[i], i);
        }
    }

    // Bind uniforms.
    ImmUniform uniforms   = NK_ZERO_MEM;
    uniforms.u_projection = g_imm.current_projection;
    uniforms.u_view       = g_imm.current_view;
    uniforms.u_model      = g_imm.current_model;
    uniforms.u_usetex     = use_texture;

    g_imm.current_uniforms[0].data = &uniforms;
    g_imm.current_uniforms[0].size = sizeof(uniforms);

    for(nkS32 i=0; i<IMM_MAX_UNIFORMS; ++i)
    {
        if(g_imm.current_uniforms[i].data)
        {
            bind_buffer(g_imm.uniform_buffers[i], i);
            update_buffer(g_imm.uniform_buffers[i], g_imm.current_uniforms[i].data, g_imm.current_uniforms[i].size);
        }
    }

    // Update data and draw.
    update_buffer(g_imm.vertex_buffer, g_imm.vertices.data, g_imm.position_count * sizeof(ImmVertex));

    draw_arrays(g_imm.position_count);

    end_render_pass();

    g_imm.draw_started = NK_FALSE;
}

GLOBAL void imm_position(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    imm_grow_vertex_array_if_necessary(g_imm.position_count+1);
    g_imm.vertices[g_imm.position_count++].position = { x,y,z,w };
}

GLOBAL void imm_normal(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    imm_grow_vertex_array_if_necessary(g_imm.normal_count+1);
    g_imm.vertices[g_imm.normal_count++].normal = { x,y,z,w };
}

GLOBAL void imm_color(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    imm_grow_vertex_array_if_necessary(g_imm.color_count+1);
    g_imm.vertices[g_imm.color_count++].color = { x,y,z,w };
}

GLOBAL void imm_texcoord(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    imm_grow_vertex_array_if_necessary(g_imm.texcoord_count+1);
    g_imm.vertices[g_imm.texcoord_count++].texcoord = { x,y,z,w };
}

GLOBAL void imm_userdata0(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    imm_grow_vertex_array_if_necessary(g_imm.userdata0_count+1);
    g_imm.vertices[g_imm.userdata0_count++].userdata0 = { x,y,z,w };
}

GLOBAL void imm_userdata1(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    imm_grow_vertex_array_if_necessary(g_imm.userdata1_count+1);
    g_imm.vertices[g_imm.userdata1_count++].userdata1 = { x,y,z,w };
}

GLOBAL void imm_userdata2(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    imm_grow_vertex_array_if_necessary(g_imm.userdata2_count+1);
    g_imm.vertices[g_imm.userdata2_count++].userdata2 = { x,y,z,w };
}

GLOBAL void imm_userdata3(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    imm_grow_vertex_array_if_necessary(g_imm.userdata3_count+1);
    g_imm.vertices[g_imm.userdata3_count++].userdata3 = { x,y,z,w };
}

// =============================================================================

// 2D Primitives ===============================================================

GLOBAL void imm_point(nkF32 x, nkF32 y, nkVec4 color)
{
    imm_begin(DrawMode_Points);
    imm_position(x,y); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_line(nkF32 x1, nkF32 y1, nkF32 x2, nkF32 y2, nkVec4 color)
{
    imm_begin(DrawMode_Lines);
    imm_position(x1,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_rect_outline(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkVec4 color)
{
    nkF32 x1 = x+0.5f;
    nkF32 y1 = y+0.5f;
    nkF32 x2 = x1+w;
    nkF32 y2 = y1+h;

    imm_begin(DrawMode_LineStrip);
    imm_position(x1,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x1,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x1,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_rect_filled(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkVec4 color)
{
    nkF32 x1 = x;
    nkF32 y1 = y;
    nkF32 x2 = x1+w;
    nkF32 y2 = y1+h;

    imm_begin(DrawMode_TriangleStrip);
    imm_position(x1,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x1,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_circle_outline(nkF32 x, nkF32 y, nkF32 r, nkS32 n, nkVec4 color)
{
    nkArray<nkVec2> points; // @Speed: Memory allocation and free for every call to this function.
    nk_array_reserve(&points, n);

    for(nkS32 i=0; i<n; ++i)
    {
        nkF32 theta = NK_TAU_F32 * NK_CAST(nkF32,i) / NK_CAST(nkF32,n);
        nkF32 xx = r * cosf(theta);
        nkF32 yy = r * sinf(theta);
        nk_array_append(&points, { x+xx,y+yy });
    }

    imm_begin(DrawMode_Lines);
    for(nkU64 i=0; i<points.length; ++i)
    {
        nkU64 j = ((i+1) % points.length);
        const nkVec2& a = points[i];
        const nkVec2& b = points[j];
        imm_position(a.x,a.y); imm_color(color.x,color.y,color.z,color.w);
        imm_position(b.x,b.y); imm_color(color.x,color.y,color.z,color.w);
    }
    imm_end();
}

GLOBAL void imm_circle_filled(nkF32 x, nkF32 y, nkF32 r, nkS32 n, nkVec4 color)
{
    nkArray<nkVec2> points; // @Speed: Memory allocation and free for every call to this function.
    nk_array_reserve(&points, n);

    nkVec2 center = { x,y };

    for(nkS32 i=0; i<n; ++i)
    {
        nkF32 theta = NK_TAU_F32 * NK_CAST(nkF32,i) / NK_CAST(nkF32,n);
        nkF32 xx = r * cosf(theta);
        nkF32 yy = r * sinf(theta);
        nk_array_append(&points, { x+xx,y+yy });
    }

    imm_begin(DrawMode_Triangles);
    for(nkU64 i=0; i<points.length; ++i)
    {
        nkU64 j = ((i+1) % points.length);
        const nkVec2& a = points[i];
        const nkVec2& b = center;
        const nkVec2& c = points[j];
        imm_position(a.x,a.y); imm_color(color.x,color.y,color.z,color.w);
        imm_position(b.x,b.y); imm_color(color.x,color.y,color.z,color.w);
        imm_position(c.x,c.y); imm_color(color.x,color.y,color.z,color.w);
    }
    imm_end();
}

// =============================================================================

// 2D Textures =================================================================

GLOBAL void imm_begin_texture_batch(Texture tex)
{
    NK_ASSERT(tex); // Need to specify an actual texture for drawing!
    imm_set_texture(tex);
    imm_begin(DrawMode_Triangles);
}

GLOBAL void imm_end_texture_batch(void)
{
    imm_end();
}

GLOBAL void imm_texture(Texture tex, nkF32 x, nkF32 y, const ImmClip* clip, nkVec4 color)
{
    if(!tex) return; // Can't do anything if we're not actually given a texture... // @Improve: Log a warning?

    nkF32 w = NK_CAST(nkF32,get_texture_width(tex));
    nkF32 h = NK_CAST(nkF32,get_texture_height(tex));

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 x1 = x-((s2-s1)*0.5f);
    nkF32 y1 = y-((t2-t1)*0.5f);
    nkF32 x2 = x1+(s2-s1);
    nkF32 y2 = y1+(t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    imm_set_texture(tex);
    imm_begin(DrawMode_TriangleStrip);
    imm_position(x1,y2); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x1,y1); imm_texcoord(s1,t1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_texcoord(s2,t2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y1); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_texture_ex(Texture tex, nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip, nkVec4 color)
{
    if(!tex) return; // Can't do anything if we're not actually given a texture... // @Improve: Log a warning?

    nkF32 w = NK_CAST(nkF32,get_texture_width(tex));
    nkF32 h = NK_CAST(nkF32,get_texture_height(tex));

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 ox = x;
    nkF32 oy = y;

    nkF32 ax = ((anchor) ? (anchor->x*(s2-s1)) : (s2-s1)*0.5f);
    nkF32 ay = ((anchor) ? (anchor->y*(t2-t1)) : (t2-t1)*0.5f);

    x -= ax;
    y -= ay;

    nkF32 x1 = 0.0f;
    nkF32 y1 = 0.0f;
    nkF32 x2 = (s2-s1);
    nkF32 y2 = (t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    nkMat4 cached_matrix = imm_get_model();
    nkMat4 model_matrix = nk_m4_identity();

    model_matrix = nk_translate(model_matrix, {   ox,   oy, 0.0f });
    model_matrix = nk_scale    (model_matrix, {   sx,   sy, 1.0f });
    model_matrix = nk_rotate   (model_matrix, { 0.0f, 0.0f, 1.0f }, angle);
    model_matrix = nk_translate(model_matrix, {  -ox,  -oy, 0.0f });
    model_matrix = nk_translate(model_matrix, {    x,    y, 0.0f });

    imm_set_model(model_matrix);
    imm_set_texture(tex);
    imm_begin(DrawMode_TriangleStrip);
    imm_position(x1,y2); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x1,y1); imm_texcoord(s1,t1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_texcoord(s2,t2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y1); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
    imm_set_model(cached_matrix);
}

GLOBAL void imm_texture_batched(nkF32 x, nkF32 y, const ImmClip* clip, nkVec4 color)
{
    if(!g_imm.current_textures[0]) return; // Need to call imm_begin_texture_batch first! // @Improve: Log a warning?

    nkF32 w = NK_CAST(nkF32, get_texture_width(g_imm.current_textures[0]));
    nkF32 h = NK_CAST(nkF32, get_texture_height(g_imm.current_textures[0]));

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 x1 = x-((s2-s1)*0.5f);
    nkF32 y1 = y-((t2-t1)*0.5f);
    nkF32 x2 = x1+(s2-s1);
    nkF32 y2 = y1+(t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    imm_position(x1,y2); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w); // BL
    imm_position(x1,y1); imm_texcoord(s1,t1); imm_color(color.x,color.y,color.z,color.w); // TL
    imm_position(x2,y1); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w); // TR
    imm_position(x2,y1); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w); // TR
    imm_position(x2,y2); imm_texcoord(s2,t2); imm_color(color.x,color.y,color.z,color.w); // BR
    imm_position(x1,y2); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w); // BL
}

GLOBAL void imm_texture_batched_ex(nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip, nkVec4 color)
{
    if(!g_imm.current_textures[0]) return; // Need to call imm_begin_texture_batch first! // @Improve: Log a warning?

    nkF32 w = NK_CAST(nkF32, get_texture_width(g_imm.current_textures[0]));
    nkF32 h = NK_CAST(nkF32, get_texture_height(g_imm.current_textures[0]));

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 ox = x;
    nkF32 oy = y;

    nkF32 ax = ((anchor) ? (anchor->x*(s2-s1)) : (s2-s1)*0.5f);
    nkF32 ay = ((anchor) ? (anchor->y*(t2-t1)) : (t2-t1)*0.5f);

    x -= ax;
    y -= ay;

    nkF32 x1 = 0.0f;
    nkF32 y1 = 0.0f;
    nkF32 x2 = (s2-s1);
    nkF32 y2 = (t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    nkMat4 model_matrix = nk_m4_identity();

    model_matrix = nk_translate(model_matrix, {   ox,   oy, 0.0f });
    model_matrix = nk_scale    (model_matrix, {   sx,   sy, 1.0f });
    model_matrix = nk_rotate   (model_matrix, { 0.0f, 0.0f, 1.0f }, angle);
    model_matrix = nk_translate(model_matrix, {  -ox,  -oy, 0.0f });
    model_matrix = nk_translate(model_matrix, {    x,    y, 0.0f });

    nkVec4 tl = { x1,y1,0.0f,1.0f };
    nkVec4 tr = { x2,y1,0.0f,1.0f };
    nkVec4 bl = { x1,y2,0.0f,1.0f };
    nkVec4 br = { x2,y2,0.0f,1.0f };

    tl = model_matrix * tl;
    tr = model_matrix * tr;
    bl = model_matrix * bl;
    br = model_matrix * br;

    imm_position(bl.x,bl.y); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w); // BL
    imm_position(tl.x,tl.y); imm_texcoord(s1,t1); imm_color(color.x,color.y,color.z,color.w); // TL
    imm_position(tr.x,tr.y); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w); // TR
    imm_position(tr.x,tr.y); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w); // TR
    imm_position(br.x,br.y); imm_texcoord(s2,t2); imm_color(color.x,color.y,color.z,color.w); // BR
    imm_position(bl.x,bl.y); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w); // BL
}

// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
