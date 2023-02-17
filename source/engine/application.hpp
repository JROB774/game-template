/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(ScreenMode, nkS32)
{
    ScreenMode_Window,       // We do not have a screen target, we just render directly into the window (default).
    ScreenMode_Fit,          // We have a screen target that resizes to match the current window size.
    ScreenMode_Stretch,      // We have a screen target that stretches to fit inside the window regardless of distortion.
    ScreenMode_Letterbox,    // We have a screen target that scales to fit the window, with letterboxing.
    ScreenMode_PixelPerfect, // We have a screen target that scales to fit the window, only in pixel-perfect increments.
    ScreenMode_TOTAL
};

struct AppDesc
{
    const nkChar* title         = "Template Game";       // Display name of the game (shown in the title bar).
    const nkChar* name          = "template";            // Internal name of the game (alphanumeric characters only).
    iPoint        window_size   = { 1280,720 };          // Starting size of the game window.
    iPoint        window_min    = { 1280,720 };          // Minimum size of the game window.
    iPoint        screen_size   = { 1280,720 };          // Size of the screen render target.
    ScreenMode    screen_mode   = ScreenMode_Window;     // How the screen should be rendered to the window.
    nkF32         tick_rate     = 60.0f;                 // How many times to run the tick callback each second.
};

// External user-defined functions!
GLOBAL void app_main(AppDesc* desc);
GLOBAL void app_init(void);
GLOBAL void app_quit(void);
GLOBAL void app_tick(nkF32 dt);
GLOBAL void app_draw(void);

/*////////////////////////////////////////////////////////////////////////////*/
