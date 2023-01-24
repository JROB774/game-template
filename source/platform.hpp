/*////////////////////////////////////////////////////////////////////////////*/

typedef void(*GameInitCallback)(void );
typedef void(*GameQuitCallback)(void );
typedef void(*GameTickCallback)(nkF32);
typedef void(*GameDrawCallback)(void );

struct GameDesc
{
    const nkChar*    title       = "Template Game"; // Display name of the game (shown in the title bar).
    const nkChar*    name        = "template";      // Internal name of the game (alphanumeric characters only).
    iPoint           window_size = { 1280,720 };    // Starting size of the game window.
    iPoint           window_min  = { 1280,720 };    // Minimum size of the game window.
    nkF32            tick_rate   = 60.0f;           // How many times to run the tick callback each second.
    GameInitCallback init        = NULL;            // Callback for initalizing the game.
    GameQuitCallback quit        = NULL;            // Callback for terminating the game.
    GameTickCallback tick        = NULL;            // Callback for updating logic every game tick.
    GameDrawCallback draw        = NULL;            // Callback for drawing the scene every frame.
};

static void entry_point(GameDesc* desc); // User-defined entry-point that can be used to fill out the GameDesc.

static nkChar* get_base_path     (void);
static void*   get_window        (void);
static void*   get_context       (void);
static void    terminate_app     (void);
static void    fatal_error       (const nkChar* fmt, ...);
static void    user_error        (const nkChar* fmt, ...);
static nkVec2  get_window_size   (void);
static nkS32   get_window_width  (void);
static nkS32   get_window_height (void);
static void    set_fullscreen    (nkBool enable);
static nkBool  is_fullscreen     (void);
static void    show_cursor       (nkBool show);
static nkBool  is_cursor_visible (void);
static nkU64   get_system_time_ms(void);
static nkU64   get_system_time_us(void);

/*////////////////////////////////////////////////////////////////////////////*/
