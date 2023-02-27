/*////////////////////////////////////////////////////////////////////////////*/

// Debug UI is only available in debug builds, so wrap ImGui code in BUILD_DEBUG!
#if defined(BUILD_DEBUG)

GLOBAL void   init_debug_ui_system   (void);
GLOBAL void   quit_debug_ui_system   (void);
GLOBAL void   process_debug_ui_events(void* event);
GLOBAL void   begin_debug_ui_frame   (void);
GLOBAL void   end_debug_ui_frame     (void);
GLOBAL void   render_debug_ui_frame  (void);
GLOBAL nkBool is_debug_ui_enabled    (void);

#else

#define init_debug_ui_system()         ((void)0 )
#define quit_debug_ui_system()         ((void)0 )
#define process_debug_ui_events(event) ((void)0 )
#define begin_debug_ui_frame()         ((void)0 )
#define end_debug_ui_frame()           ((void)0 )
#define render_debug_ui_frame()        ((void)0 )
#define is_debug_ui_enabled()          (NK_FALSE)

#endif // BUILD_DEBUG

/*////////////////////////////////////////////////////////////////////////////*/
