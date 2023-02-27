/*////////////////////////////////////////////////////////////////////////////*/

#if defined(BUILD_DEBUG)

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#define IMGUI_DEFINE_MATH_OPERATORS

#if defined(BUILD_WEB)
#define IMGUI_IMPL_OPENGL_ES3
#endif // BUILD_WEB

#include <imgui.h>

#include <backends/imgui_impl_sdl2.cpp>
#include <backends/imgui_impl_opengl3.cpp>

#include <imgui.cpp>
#include <imgui_widgets.cpp>
#include <imgui_demo.cpp>
#include <imgui_draw.cpp>
#include <imgui_tables.cpp>

struct DebugUiContext
{
    nkBool enabled;
};

INTERNAL DebugUiContext g_debug_ui;

GLOBAL void init_debug_ui_system(void)
{
    // Currently we are just hard-coding this to use SDL and OpenGL backends
    // but if we start supporting more backends in the future then we probably
    // want to reorganize and clean this stuff up to handle things better.

    // Setup ImGui.
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    SDL_Window* window = NK_CAST(SDL_Window*,get_window());
    SDL_GLContext context = NK_CAST(SDL_GLContext,get_context());

    ImGui_ImplSDL2_InitForOpenGL(window, context);

    #if defined(BUILD_NATIVE)
    #if defined(BUILD_DEBUG) && defined(NK_OS_WIN32)
    ImGui_ImplOpenGL3_Init("#version 430");
    #else
    ImGui_ImplOpenGL3_Init("#version 330");
    #endif // BUILD_DEBUG && NK_OS_WIN32
    #endif // BUILD_NATIVE
    #if defined(BUILD_WEB)
    ImGui_ImplOpenGL3_Init("#version 300 es");
    #endif // BUILD_WEB

}

GLOBAL void quit_debug_ui_system(void)
{
    ImGui_ImplOpenGL3_Shutdown();
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
    ImGui_ImplOpenGL3_NewFrame();

    ImGui::NewFrame();
}

GLOBAL void end_debug_ui_frame(void)
{
    ImGui::Render();
}

GLOBAL void render_debug_ui_frame(void)
{
    ImDrawData* imgui_draw_data = ImGui::GetDrawData();
    if(imgui_draw_data)
    {
        ImGui_ImplOpenGL3_RenderDrawData(imgui_draw_data);
    }
}

GLOBAL nkBool is_debug_ui_enabled(void)
{
    return g_debug_ui.enabled;
}

#endif // BUILD_DEBUG

/*////////////////////////////////////////////////////////////////////////////*/
