/*////////////////////////////////////////////////////////////////////////////*/

#if defined(BUILD_WEB)
#include <emscripten.h>
#endif // BUILD_WEB

#include <SDL.h>
#include <SDL_mixer.h>

// On Windows we are using Direct3D so we do not want the OpenGL flag specified.
#if defined(NK_OS_WIN32)
INTERNAL constexpr nkU32 WINDOW_FLAGS = SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE;
#else
INTERNAL constexpr nkU32 WINDOW_FLAGS = SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL;
#endif

INTERNAL constexpr const nkChar* PROGRAM_STATE_FILE = "state.dat";

#pragma pack(push,1)
struct ProgramState
{
    nkF32  sound_volume;
    nkF32  music_volume;
    nkS32  window_x;
    nkS32  window_y;
    nkS32  window_w;
    nkS32  window_h;
    nkS32  display;
    nkBool fullscreen;
    nkBool maximized;
};
#pragma pack(pop)

struct PlatformContext
{
    AppDesc       app_desc;
    SDL_Window*   window;
    Texture       screen;
    nkBool        running;
    nkChar*       base_path;
    nkS32         window_x;
    nkS32         window_y;
    nkS32         window_w;
    nkS32         window_h;
    nkBool        maximized;
    nkBool        fullscreen;
    nkU64         ticks;
};

INTERNAL PlatformContext g_ctx;

INTERNAL void cache_window_bounds(void)
{
    if(g_ctx.maximized) SDL_RestoreWindow(g_ctx.window);
    SDL_GetWindowPosition(g_ctx.window, &g_ctx.window_x, &g_ctx.window_y);
    SDL_GetWindowSize(g_ctx.window, &g_ctx.window_w, &g_ctx.window_h);
    if(g_ctx.maximized) SDL_MaximizeWindow(g_ctx.window);
}

INTERNAL void load_program_state(void)
{
    nkChar file_name[1024] = NK_ZERO_MEM;

    strcpy(file_name, get_base_path());
    strcat(file_name, PROGRAM_STATE_FILE);

    if(!nk_file_exists(file_name)) return;

    // Load the program state from disk.
    ProgramState program_state = NK_ZERO_MEM;

    FILE* file = fopen(file_name, "rb");
    if(!file)
    {
        printf("Failed to open program state file: %s\n", file_name);
        return;
    }
    size_t result = fread(&program_state, sizeof(program_state), 1, file);
    fclose(file);
    if(result != 1)
    {
        printf("Failed to read program state file: %s\n", file_name);
        return;
    }

    // Set the program state values.
    set_sound_volume(program_state.sound_volume);
    set_music_volume(program_state.music_volume);

    SDL_Rect display_bounds;
    if(SDL_GetDisplayBounds(program_state.display, &display_bounds) < 0)
    {
        printf("Previous display no longer available, aborting window bounds restore!\n");
    }
    else
    {
        // Make sure the window is not out of bounds at all.
        if(program_state.window_x != SDL_WINDOWPOS_CENTERED && program_state.window_x < display_bounds.x)
            program_state.window_x = display_bounds.x;
        if(program_state.window_x != SDL_WINDOWPOS_CENTERED && (program_state.window_x+program_state.window_w) >= display_bounds.w)
            program_state.window_x = display_bounds.w - program_state.window_w;
        if(program_state.window_y != SDL_WINDOWPOS_CENTERED && program_state.window_y < display_bounds.y)
            program_state.window_y = display_bounds.y;
        if(program_state.window_y != SDL_WINDOWPOS_CENTERED && (program_state.window_y+program_state.window_h) >= display_bounds.h)
            program_state.window_y = display_bounds.h - program_state.window_h;
    }

    SDL_SetWindowSize(g_ctx.window, program_state.window_w,program_state.window_h);
    SDL_SetWindowPosition(g_ctx.window, program_state.window_x,program_state.window_y);

    // We don't set the context's maximized state here, we instead do it in the
    // event loop so that it also gets set when the maximize button is toggled.
    if(program_state.maximized) SDL_MaximizeWindow(g_ctx.window);

    set_fullscreen(program_state.fullscreen);
}

INTERNAL void save_program_state(void)
{
    // Set the program state values.
    ProgramState program_state = NK_ZERO_MEM;

    program_state.sound_volume = get_sound_volume();
    program_state.music_volume = get_music_volume();

    SDL_RestoreWindow(g_ctx.window);

    if(!is_fullscreen())
    {
        SDL_GetWindowPosition(g_ctx.window, &program_state.window_x,&program_state.window_y);
        SDL_GetWindowSize(g_ctx.window, &program_state.window_w,&program_state.window_h);
    }
    else
    {
        // Use the cached bounds as they represent the actual window pos and size.
        program_state.window_x = g_ctx.window_x;
        program_state.window_y = g_ctx.window_y;
        program_state.window_w = g_ctx.window_w;
        program_state.window_h = g_ctx.window_h;
    }

    program_state.maximized = g_ctx.maximized;
    program_state.fullscreen = g_ctx.fullscreen;

    program_state.display = SDL_GetWindowDisplayIndex(g_ctx.window);

    // Write the program state to disk.
    nkChar file_name[1024] = NK_ZERO_MEM;

    strcpy(file_name, get_base_path());
    strcat(file_name, PROGRAM_STATE_FILE);

    FILE* file = fopen(file_name, "wb");
    if(!file)
    {
        printf("Failed to open program state file: %s\n", file_name);
        return;
    }
    size_t result = fwrite(&program_state, sizeof(program_state), 1, file);
    fclose(file);
    if(result != 1)
    {
        printf("Failed to write program state file: %s\n", file_name);
        return;
    }
}

INTERNAL void begin_render_frame(void)
{
    nkS32 iww = get_window_width();
    nkS32 iwh = get_window_height();
    nkF32 fww = NK_CAST(nkF32, iww);
    nkF32 fwh = NK_CAST(nkF32, iwh);

    nkS32 isw = get_texture_width(g_ctx.screen);
    nkS32 ish = get_texture_height(g_ctx.screen);
    nkF32 fsw = NK_CAST(nkF32, isw);
    nkF32 fsh = NK_CAST(nkF32, ish);

    switch(g_ctx.app_desc.screen_mode)
    {
        case ScreenMode_Window:
        {
            set_viewport(0.0f,0.0f,fww,fwh);
        } break;

        case ScreenMode_Fit:
        {
            if(iww != isw || iwh != ish)
                resize_texture(g_ctx.screen, iww,iwh); // The screen target should always fit the window.
            set_viewport(0.0f,0.0f,fww,fwh);
        } break;

        case ScreenMode_Stretch:
        case ScreenMode_Letterbox:
        case ScreenMode_PixelPerfect:
        {
            set_viewport(0.0f,0.0f,fsw,fsh);
        } break;
    }

    imm_clear(0.0f, 0.0f, 0.0f);
}

INTERNAL void end_render_frame(void)
{
    // Perform post-processing if there are effects to perform.
    Texture screen = g_ctx.screen;
    if((num_post_process_effects() > 0) && (g_ctx.app_desc.screen_mode != ScreenMode_Window))
    {
        screen = perform_post_processing(screen);
    }

    // Draw the screen to the window.
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkF32 sw = NK_CAST(nkF32, get_texture_width(screen));
    nkF32 sh = NK_CAST(nkF32, get_texture_height(screen));

    imm_reset();

    switch(g_ctx.app_desc.screen_mode)
    {
        case ScreenMode_Window:
        {
            // Do nothing as we have already been drawing to the window...
        } break;

        case ScreenMode_Fit:
        case ScreenMode_Stretch:
        {
            imm_set_viewport(0.0f,0.0f,ww,wh);
            imm_set_color_target(BACKBUFFER);
            imm_set_texture(screen);
            imm_set_sampler(imm_get_def_sampler(g_ctx.app_desc.screen_filter));
            imm_begin(DrawMode_TriangleStrip);
            imm_position(0.0f,  wh); imm_texcoord(0.0f,0.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_position(0.0f,0.0f); imm_texcoord(0.0f,1.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_position(  ww,  wh); imm_texcoord(1.0f,0.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_position(  ww,0.0f); imm_texcoord(1.0f,1.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_end();
        } break;

        case ScreenMode_Letterbox:
        {
            nkF32 sx = ww / sw;
            nkF32 sy = wh / sh;

            nkF32 dstw = sw;
            nkF32 dsth = sh;

            if(fabsf(sx) < fabsf(sy)) dsth = roundf((sh/sw)*dstw);
            if(fabsf(sx) > fabsf(sy)) dstw = roundf((sw/sh)*dsth);

            nkF32 scale = nk_min(ww / dstw, wh / dsth);

            nkF32 vw = (dstw * scale);
            nkF32 vh = (dsth * scale);
            nkF32 vx = (ww - vw) * 0.5f;
            nkF32 vy = (wh - vh) * 0.5f;

            imm_set_viewport(0.0f,0.0f,ww,wh);
            imm_set_color_target(BACKBUFFER);
            imm_set_texture(screen);
            imm_set_sampler(imm_get_def_sampler(g_ctx.app_desc.screen_filter));
            imm_begin(DrawMode_TriangleStrip);
            imm_position(vx,   vy+vh); imm_texcoord(0.0f,0.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_position(vx,   vy   ); imm_texcoord(0.0f,1.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_position(vx+vw,vy+vh); imm_texcoord(1.0f,0.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_position(vx+vw,vy   ); imm_texcoord(1.0f,1.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_end();
        } break;

        case ScreenMode_PixelPerfect:
        {
            nkF32 dstw = sw;
            nkF32 dsth = sh;

            while((dstw+sw <= ww) && (dsth+sh <= wh))
            {
                dstw += sw;
                dsth += sh;
            }

            nkF32 vw = dstw;
            nkF32 vh = dsth;
            nkF32 vx = (ww - vw) * 0.5f;
            nkF32 vy = (wh - vh) * 0.5f;

            imm_set_viewport(0.0f,0.0f,ww,wh);
            imm_set_color_target(BACKBUFFER);
            imm_set_texture(screen);
            imm_set_sampler(imm_get_def_sampler(g_ctx.app_desc.screen_filter));
            imm_begin(DrawMode_TriangleStrip);
            imm_position(vx,   vy+vh); imm_texcoord(0.0f,0.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_position(vx,   vy   ); imm_texcoord(0.0f,1.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_position(vx+vw,vy+vh); imm_texcoord(1.0f,0.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_position(vx+vw,vy   ); imm_texcoord(1.0f,1.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
            imm_end();
        } break;
    }
}

INTERNAL void main_init(void)
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fatal_error("Failed to initialize SDL systems: %s", SDL_GetError());
    }

    setup_renderer_platform();

    g_ctx.window = SDL_CreateWindow(g_ctx.app_desc.title, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        g_ctx.app_desc.window_size.x,g_ctx.app_desc.window_size.y, WINDOW_FLAGS);
    if(!g_ctx.window)
        fatal_error("Failed to create application window: %s", SDL_GetError());
    SDL_SetWindowMinimumSize(g_ctx.window, g_ctx.app_desc.window_min.x,g_ctx.app_desc.window_min.y);

    g_ctx.base_path = SDL_GetBasePath();

    init_render_system();
    init_post_process_system();
    init_audio_system();
    init_asset_manager();
    init_truetype_font_system();
    init_input_system();
    init_debug_ui_system();

    imm_init();

    // Create the screen render target texture.
    TextureDesc td;
    td.type   = TextureType_2D;
    td.format = TextureFormat_RGBA;
    td.width  = g_ctx.app_desc.screen_size.x;
    td.height = g_ctx.app_desc.screen_size.y;
    td.data   = NULL;
    g_ctx.screen = create_texture(td);

    app_init();

    g_ctx.running = NK_TRUE;
}

INTERNAL void main_quit(void)
{
    app_quit();

    free_texture(g_ctx.screen);

    imm_quit();

    quit_asset_manager(); // Shutdown first as freeing assets may depend on other systems being around!
    quit_debug_ui_system();
    quit_input_system();
    quit_truetype_font_system();
    quit_audio_system();
    quit_post_process_system();
    quit_render_system();

    SDL_free(g_ctx.base_path);

    SDL_DestroyWindow(g_ctx.window);

    SDL_Quit();
}

INTERNAL void main_loop(void)
{
    PERSISTENT nkU64 perf_frequency  = 0;
    PERSISTENT nkU64 last_counter    = 0;
    PERSISTENT nkU64 end_counter     = 0;
    PERSISTENT nkU64 elapsed_counter = 0;
    PERSISTENT nkF32 update_timer    = 0.0f;

    nkF32 dt = 1.0f / g_ctx.app_desc.tick_rate; // We use a fixed update rate to keep things deterministic.

    if(perf_frequency == 0)
    {
        perf_frequency = SDL_GetPerformanceFrequency();
        last_counter = SDL_GetPerformanceCounter();
    }

    // Just confirm our fullscreen state is accurate.
    g_ctx.fullscreen = NK_CHECK_FLAGS(SDL_GetWindowFlags(g_ctx.window), SDL_WINDOW_FULLSCREEN|SDL_WINDOW_FULLSCREEN);

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        process_input_events(&event);
        process_debug_ui_events(&event);

        switch(event.type)
        {
            case(SDL_KEYDOWN):
            {
                switch(event.key.keysym.sym)
                {
                    case(SDLK_RETURN): if(!(SDL_GetModState()&KMOD_ALT)) break;
                    case(SDLK_F11):
                    {
                        set_fullscreen(!is_fullscreen());
                    } break;
                }
            } break;
            case SDL_WINDOWEVENT:
            {
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_MAXIMIZED: g_ctx.maximized = NK_TRUE; break;
                    case SDL_WINDOWEVENT_RESTORED: g_ctx.maximized = NK_FALSE; break;
                }
            } break;
            case(SDL_QUIT):
            {
                stop_music(); // Music can cause some blocking scenarios if not stopped, so just make sure it is before quitting.
                g_ctx.running = NK_FALSE;
            } break;
        }
    }

    while(update_timer >= dt)
    {
        update_input_state();

        begin_debug_ui_frame();

        clear_post_process_effects(); // We clear effects before each tick.

        app_tick(dt);

        end_debug_ui_frame();
        reset_input_state();

        g_ctx.ticks++;

        update_timer -= dt;
    }

    maybe_resize_backbuffer();

    imm_begin_frame();
    begin_render_frame();

    app_draw();

    end_render_frame();
    imm_end_frame();

    render_debug_ui_frame();

    present_renderer();

    end_counter = SDL_GetPerformanceCounter();
    elapsed_counter = end_counter - last_counter;
    last_counter = SDL_GetPerformanceCounter();

    nkF32 elapsed_time = NK_CAST(nkF32,elapsed_counter) / NK_CAST(nkF32,perf_frequency);

    update_timer += elapsed_time;

    #if defined(BUILD_DEBUG)
    nkF32 current_fps = NK_CAST(nkF32,perf_frequency) / NK_CAST(nkF32,elapsed_counter);
    nkChar title_buffer[1024] = NK_ZERO_MEM;
    snprintf(title_buffer, NK_ARRAY_SIZE(title_buffer), "%s (FPS: %f)", g_ctx.app_desc.title, current_fps);
    SDL_SetWindowTitle(g_ctx.window, title_buffer);
    #endif // BUILD_DEBUG

    // The window starts out hidden, after the first draw we unhide the window as this looks quite clean.
    if(NK_CHECK_FLAGS(SDL_GetWindowFlags(g_ctx.window), SDL_WINDOW_HIDDEN))
    {
        SDL_ShowWindow(g_ctx.window);
    }
}

// Different main function flow for native desktop and web builds.

#if defined(BUILD_NATIVE)
int main(int argc, char** argv)
{
    app_main(&g_ctx.app_desc);

    main_init();
    load_program_state();
    while(g_ctx.running)
        main_loop();
    save_program_state();
    main_quit();

    return 0;
}
#endif // BUILD_NATIVE

#if defined(BUILD_WEB)
INTERNAL SDL_atomic_t g_idbfs_ready;

extern "C"
{
    void filesystem_ready(void)
    {
        SDL_AtomicSet(&g_idbfs_ready, 1);
        printf("[Platform]: IDBFS ready!\n");
    }

    void main_loop_wrapper(void)
    {
        PERSISTENT nkBool initialized = NK_FALSE;

        if(!SDL_AtomicGet(&g_idbfs_ready)) return;

        if(!initialized)
        {
            initialized = NK_TRUE;
            main_init();
        }

        main_loop();
    }
}

int main(int argc, char** argv)
{
    printf("[Platform]: Setting up IDBFS...\n");

    app_main(&g_ctx.app_desc);

    EM_ASM_
    ({
        FS.mkdir(Module.UTF8ToString($0));
        FS.mount(IDBFS, {}, Module.UTF8ToString($0));
        FS.syncfs(true, function(err)
        {
            assert(!err);
            ccall("filesystem_ready");
        });
    }, g_ctx.app_desc.name);

    emscripten_set_main_loop(main_loop_wrapper, -1, 1);

    return 0;
}
#endif // BUILD_WEB

/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL AppDesc* get_app_desc(void)
{
    return &g_ctx.app_desc;
}

GLOBAL nkChar* get_base_path(void)
{
    return g_ctx.base_path;
}

GLOBAL void* get_window(void)
{
    return g_ctx.window;
}

GLOBAL void terminate_app(void)
{
    g_ctx.running = NK_FALSE;
}

GLOBAL void fatal_error(const nkChar* fmt, ...)
{
    nkChar message_buffer[1024] = NK_ZERO_MEM;

    va_list args;

    va_start(args, fmt);
    vsnprintf(message_buffer, NK_ARRAY_SIZE(message_buffer), fmt, args);
    va_end(args);

    #if defined(BUILD_DEBUG)
    fprintf(stderr, "%s\n", message_buffer);
    #endif // BUILD_DEBUG

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message_buffer, g_ctx.window);

    abort();
}

GLOBAL void user_error(const nkChar* fmt, ...)
{
    nkChar message_buffer[1024] = NK_ZERO_MEM;

    va_list args;

    va_start(args, fmt);
    vsnprintf(message_buffer, NK_ARRAY_SIZE(message_buffer), fmt, args);
    va_end(args);

    #if defined(BUILD_DEBUG)
    fprintf(stderr, "%s\n", message_buffer);
    #endif // BUILD_DEBUG

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message_buffer, g_ctx.window);
}

GLOBAL iPoint get_window_size(void)
{
    nkS32 width,height;
    SDL_GetWindowSize(g_ctx.window, &width,&height);
    iPoint size;
    size.x = width;
    size.y = height;
    return size;
}

GLOBAL nkS32 get_window_width(void)
{
    return get_window_size().x;
}

GLOBAL nkS32 get_window_height(void)
{
    return get_window_size().y;
}

GLOBAL Texture get_screen(void)
{
    return ((g_ctx.app_desc.screen_mode == ScreenMode_Window) ? BACKBUFFER : g_ctx.screen);
}

GLOBAL iPoint get_screen_size(void)
{
    if(g_ctx.app_desc.screen_mode == ScreenMode_Window)
        return get_window_size();
    iPoint size;
    size.x = get_texture_width(g_ctx.screen);
    size.y = get_texture_height(g_ctx.screen);
    return size;
}

GLOBAL nkS32 get_screen_width(void)
{
    return get_screen_size().x;
}

GLOBAL nkS32 get_screen_height(void)
{
    return get_screen_size().y;
}

GLOBAL void set_fullscreen(nkBool enable)
{
    if(g_ctx.fullscreen == enable) return;
    if(enable) cache_window_bounds();
    SDL_SetWindowFullscreen(g_ctx.window, (enable) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    g_ctx.fullscreen = enable;
}

GLOBAL nkBool is_fullscreen(void)
{
    return g_ctx.fullscreen;
}

GLOBAL void show_cursor(nkBool show)
{
    SDL_ShowCursor((show ? SDL_ENABLE : SDL_DISABLE));
}

GLOBAL nkBool is_cursor_visible()
{
    return (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
}

GLOBAL nkU64 get_system_time_ms()
{
    nkF64 freq = NK_CAST(nkF64, SDL_GetPerformanceFrequency());
    nkF64 tick = NK_CAST(nkF64, SDL_GetPerformanceCounter());

    nkF64 ms = tick * (1.0 / (freq / 1000.0));

    return NK_CAST(nkU64, ms);
}

GLOBAL nkU64 get_system_time_us()
{
    nkF64 freq = NK_CAST(nkF64, SDL_GetPerformanceFrequency());
    nkF64 tick = NK_CAST(nkF64, SDL_GetPerformanceCounter());

    nkF64 us = tick * ((1.0 / (freq / 1000.0)) * 1000.0);

    return NK_CAST(nkU64, us);
}

GLOBAL nkU64 get_elapsed_ticks(void)
{
    return g_ctx.ticks;
}

/*////////////////////////////////////////////////////////////////////////////*/
