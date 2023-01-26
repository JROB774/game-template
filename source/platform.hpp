/*////////////////////////////////////////////////////////////////////////////*/

static nkChar* get_base_path     (void);
static void*   get_window        (void);
static void*   get_context       (void);
static void    terminate_app     (void);
static void    fatal_error       (const nkChar* fmt, ...);
static void    user_error        (const nkChar* fmt, ...);
static iPoint  get_window_size   (void);
static nkS32   get_window_width  (void);
static nkS32   get_window_height (void);
static void    set_fullscreen    (nkBool enable);
static nkBool  is_fullscreen     (void);
static void    show_cursor       (nkBool show);
static nkBool  is_cursor_visible (void);
static nkU64   get_system_time_ms(void);
static nkU64   get_system_time_us(void);

/*////////////////////////////////////////////////////////////////////////////*/
