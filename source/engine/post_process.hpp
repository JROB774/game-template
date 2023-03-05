/*////////////////////////////////////////////////////////////////////////////*/

// =============================================================================
// !!! NOTE: Do NOT set post-process effects in app_draw, only in app_tick! !!!
// =============================================================================

struct PostProcessEffect
{
    const nkChar* name          = NULL;
    Shader        effect        = NULL;
    Texture       textures[8]   = {};
    Sampler       samplers[8]   = {};
    Sampler       input_sampler = {};
    void*         uniforms      = NULL;
    nkU64         uniform_bytes = 0;
    nkS32         output_width  = 0; // 0 == same as the input width.
    nkS32         output_height = 0; // 0 == same as the input height.
};

GLOBAL void    init_post_process_system  (void);
GLOBAL void    quit_post_process_system  (void);
GLOBAL void    push_post_process_effect  (const PostProcessEffect& effect);
GLOBAL void    pop_post_process_effect   (void);
GLOBAL void    clear_post_process_effects(void);
GLOBAL nkU64   num_post_process_effects  (void);
GLOBAL Texture perform_post_processing   (Texture input);

/*////////////////////////////////////////////////////////////////////////////*/
