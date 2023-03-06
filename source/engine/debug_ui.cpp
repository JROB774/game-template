/*////////////////////////////////////////////////////////////////////////////*/

// =============================================================================
// We implement our own rendering code for ImGui that makes use of our rendering
// API. This is preferred as it will neatly wrap whatever graphics API we want
// to use on a given platform without having to juggle different ImGui backend
// implementation files.
//
// This rendering implementation is based off of imgui_impl_opengl3.
// =============================================================================

#if defined(BUILD_DEBUG)

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>

#include <backends/imgui_impl_sdl2.cpp>

#include <imgui.cpp>
#include <imgui_widgets.cpp>
#include <imgui_demo.cpp>
#include <imgui_draw.cpp>
#include <imgui_tables.cpp>

struct ImGuiUniforms
{
    nkMat4 projection;
};

struct ImGuiDrawData
{
    VertexLayout   vertex_layout;
    RenderPass     render_pass;
    RenderPipeline render_pipeline;
    Buffer         vertex_buffer;
    Buffer         index_buffer;
    Buffer         uniform_buffer;
    Shader         shader;
    Texture        font_texture;
    nkBool         initialized;
};

struct DebugUiContext
{
    ImGuiDrawData draw_data;
    nkBool        enabled;
};

INTERNAL DebugUiContext g_debug_ui;

INTERNAL ImGuiDrawData* get_imgui_draw_data(void)
{
    return ((ImGui::GetCurrentContext()) ? NK_CAST(ImGuiDrawData*,ImGui::GetIO().BackendRendererUserData) : NULL);
}

INTERNAL void create_draw_data_resources(ImGuiDrawData* draw_data)
{
    NK_ASSERT(draw_data);

    // Load the shader.
    draw_data->shader = asset_manager_load<Shader>("imgui.shader");

    // Create the vertex layout.
    draw_data->vertex_layout.attribs[0]   = { 0, AttribType_Float,        2, IM_OFFSETOF(ImDrawVert, pos), NK_TRUE };
    draw_data->vertex_layout.attribs[1]   = { 1, AttribType_Float,        2, IM_OFFSETOF(ImDrawVert, uv),  NK_TRUE };
    draw_data->vertex_layout.attribs[2]   = { 2, AttribType_UnsignedByte, 4, IM_OFFSETOF(ImDrawVert, col), NK_TRUE };
    draw_data->vertex_layout.attrib_count = 3;
    draw_data->vertex_layout.byte_stride  = sizeof(ImDrawVert);

    // Create the render pass.
    RenderPassDesc pass;
    pass.color_targets[0]     = BACKBUFFER;
    pass.depth_stencil_target = NULL;
    pass.num_color_targets    = 1;
    pass.clear                = NK_FALSE;

    draw_data->render_pass = create_render_pass(pass);

    // Create the render pipeline.
    RenderPipelineDesc pipe;
    pipe.vertex_layout = draw_data->vertex_layout;
    pipe.render_pass   = draw_data->render_pass;
    pipe.shader        = draw_data->shader;
    pipe.draw_mode     = DrawMode_Triangles;
    pipe.blend_mode    = BlendMode_Alpha;
    pipe.cull_face     = CullFace_None;
    pipe.depth_read    = NK_FALSE;
    pipe.depth_write   = NK_FALSE;

    draw_data->render_pipeline = create_render_pipeline(pipe);

    // Create the buffers.
    BufferDesc vb;
    vb.type  = BufferType_Vertex;
    vb.usage = BufferUsage_Stream;
    vb.data  = NULL;
    vb.bytes = 0;

    BufferDesc ib;
    ib.type  = BufferType_Element;
    ib.usage = BufferUsage_Stream;
    ib.data  = NULL;
    ib.bytes = 0;

    BufferDesc ub;
    ub.type  = BufferType_Uniform;
    ub.usage = BufferUsage_Stream;
    ub.data  = NULL;
    ub.bytes = 0;

    draw_data->vertex_buffer  = create_buffer(vb);
    draw_data->index_buffer   = create_buffer(ib);
    draw_data->uniform_buffer = create_buffer(ub);

    // Create the font atlas texture.
    ImGuiIO& io = ImGui::GetIO();

    nkS32 width, height;
    nkU8* pixels;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    TextureDesc td;
    td.type   = TextureType_2D;
    td.format = TextureFormat_RGBA;
    td.width  = width;
    td.height = height;
    td.data   = NK_CAST(void*,pixels);

    draw_data->font_texture = create_texture(td);

    io.Fonts->SetTexID(NK_CAST(ImTextureID, NK_CAST(intptr_t,draw_data->font_texture)));

    draw_data->initialized = NK_TRUE;
}

INTERNAL void destroy_draw_data_resources(ImGuiDrawData* draw_data)
{
    NK_ASSERT(draw_data);

    free_texture(draw_data->font_texture);
    free_buffer(draw_data->uniform_buffer);
    free_buffer(draw_data->index_buffer);
    free_buffer(draw_data->vertex_buffer);
    free_render_pipeline(draw_data->render_pipeline);
    free_render_pass(draw_data->render_pass);

    draw_data->initialized = NK_FALSE;
}

GLOBAL void init_debug_ui_system(void)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    SDL_Window* window = NK_CAST(SDL_Window*,get_window());
    SDL_GLContext context = NK_CAST(SDL_GLContext,get_context());

    ImGui_ImplSDL2_InitForOpenGL(window, context);

    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->SetTexID(0);

    io.BackendRendererUserData = NK_CAST(void*, &g_debug_ui.draw_data);
    io.BackendRendererName     = "ImGui Renderer";
}

GLOBAL void quit_debug_ui_system(void)
{
    ImGuiDrawData* draw_data = get_imgui_draw_data();
    IM_ASSERT(draw_data != NULL && "No renderer backend to shutdown, or already shutdown?");

    destroy_draw_data_resources(draw_data);

    ImGuiIO& io = ImGui::GetIO();

    io.BackendRendererName     = NULL;
    io.BackendRendererUserData = NULL;

    ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext();
}

GLOBAL void process_debug_ui_events(void* event)
{
    SDL_Event* sdl_event = NK_CAST(SDL_Event*,event);
    ImGui_ImplSDL2_ProcessEvent(sdl_event);
}

GLOBAL void begin_debug_ui_frame(void)
{
    if(is_key_pressed(KeyCode_F1))
    {
        g_debug_ui.enabled = !g_debug_ui.enabled;
    }

    ImGui_ImplSDL2_NewFrame();

    ImGuiDrawData* draw_data = get_imgui_draw_data();
    IM_ASSERT(draw_data != NULL && "Draw data not set?");

    // If we haven't been initialized yet then do so now.
    if(!draw_data->initialized)
    {
        create_draw_data_resources(draw_data);
    }

    ImGui::NewFrame();
}

GLOBAL void end_debug_ui_frame(void)
{
    ImGui::Render();
}

GLOBAL void render_debug_ui_frame(void)
{
    ImDrawData* imgui_draw_data = ImGui::GetDrawData();
    if(!imgui_draw_data) return;

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates).
    nkS32 fb_width = NK_CAST(nkS32, (imgui_draw_data->DisplaySize.x * imgui_draw_data->FramebufferScale.x));
    nkS32 fb_height = NK_CAST(nkS32, (imgui_draw_data->DisplaySize.y * imgui_draw_data->FramebufferScale.y));

    if(fb_width <= 0 || fb_height <= 0) return;

    ImGuiDrawData* draw_data = get_imgui_draw_data();

    nkF32 vw = NK_CAST(nkF32,fb_width);
    nkF32 vh = NK_CAST(nkF32,fb_height);

    nkF32 l = imgui_draw_data->DisplayPos.x;
    nkF32 r = imgui_draw_data->DisplayPos.x + imgui_draw_data->DisplaySize.x;
    nkF32 t = imgui_draw_data->DisplayPos.y;
    nkF32 b = imgui_draw_data->DisplayPos.y + imgui_draw_data->DisplaySize.y;

    ImGuiUniforms uniforms = NK_ZERO_MEM;
    uniforms.projection = nk_orthographic(l,r,b,t);

    // Will project scissor/clipping rectangles into framebuffer space.
    ImVec2 clip_off   = imgui_draw_data->DisplayPos;       // (0,0) unless using multi-viewports.
    ImVec2 clip_scale = imgui_draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2).

    set_viewport(0.0f,0.0f,vw,vh);

    begin_render_pass(draw_data->render_pass);

    bind_pipeline(draw_data->render_pipeline);

    bind_buffer(draw_data->vertex_buffer);
    bind_buffer(draw_data->index_buffer);
    bind_buffer(draw_data->uniform_buffer);

    update_buffer(draw_data->uniform_buffer, &uniforms, sizeof(uniforms));

    // Render command lists.
    for(nkS32 i=0; i<imgui_draw_data->CmdListsCount; ++i)
    {
        const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];

        nkU64 vtx_buffer_size = NK_CAST(nkU64, cmd_list->VtxBuffer.Size * NK_CAST(nkS32,sizeof(ImDrawVert)));
        nkU64 idx_buffer_size = NK_CAST(nkU64, cmd_list->IdxBuffer.Size * NK_CAST(nkS32,sizeof(ImDrawIdx)));

        update_buffer(draw_data->vertex_buffer, cmd_list->VtxBuffer.Data, vtx_buffer_size);
        update_buffer(draw_data->index_buffer, cmd_list->IdxBuffer.Data, idx_buffer_size);

        for(nkS32 cmd_i=0; cmd_i<cmd_list->CmdBuffer.Size; ++cmd_i)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            if(pcmd->UserCallback)
            {
                // NOTE: We aren;t handling ImDrawCallback_ResetRenderState, maybe we should?
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space.
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                if(clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) continue;

                // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                nkF32 sx = clip_min.x;
                nkF32 sy = clip_min.y;
                nkF32 sw = clip_max.x - clip_min.x;
                nkF32 sh = clip_max.y - clip_min.y;

                begin_scissor(sx,sy,sw,sh);

                bind_texture(NK_CAST(Texture,pcmd->GetTexID()), imm_get_def_sampler(ImmSampler_ClampLinear), 0);

                ElementType element_type = ((sizeof(ImDrawIdx) == 2) ? ElementType_UnsignedShort : ElementType_UnsignedInt);
                draw_elements(pcmd->ElemCount, element_type, pcmd->IdxOffset * sizeof(ImDrawIdx));

                end_scissor();
            }
        }
    }

    end_render_pass();
}

GLOBAL nkBool is_debug_ui_enabled(void)
{
    return g_debug_ui.enabled;
}

#endif // BUILD_DEBUG

/*////////////////////////////////////////////////////////////////////////////*/
