/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(ImmSampler, nkS32)
{
    ImmSampler_ClampNearest,
    ImmSampler_ClampLinear,
    ImmSampler_RepeatNearest,
    ImmSampler_RepeatLinear,
    ImmSampler_TOTAL
};

// System functions that should already be getting called, no need to use these!
GLOBAL void imm_init       (void);
GLOBAL void imm_quit       (void);
GLOBAL void imm_begin_frame(void);
GLOBAL void imm_end_frame  (void);

// General =====================================================================
GLOBAL void imm_clear(nkVec4 color);
GLOBAL void imm_clear(nkVec3 color);
GLOBAL void imm_clear(nkF32 r, nkF32 g, nkF32 b, nkF32 a = 1.0f);
GLOBAL void imm_reset(void); // Reset all of the imm state values back to their defaults.
// =============================================================================

// State Setters ===============================================================
GLOBAL void imm_set_color_target(Texture target);                      // Set a color target to use for rendering (NULL or BACKBUFFER for the backbuffer).
GLOBAL void imm_set_depth_target(Texture target);                      // Set a depth target to use for rendering (NULL for none).
GLOBAL void imm_set_uniforms    (void* data, nkU64 bytes, nkU32 slot); // Set some custom uniform data to use for rendering, set to NULL to disable. (Data must survive until a call to imm_end).
GLOBAL void imm_set_shader      (Shader shader);                       // Set a shader to use for rendering, set to NULL to use the built-in immediate mode shader.
GLOBAL void imm_set_sampler     (Sampler sampler, nkS32 slot = 0);     // Set a sampler to use for rendering, set to NULL to use the built-in immediate mode sampler.
GLOBAL void imm_set_texture     (Texture texture, nkS32 slot = 0);     // Set a texture to use for rendering, set to NULL for no texture to be used.
GLOBAL void imm_set_viewport    (nkF32 x, nkF32 y, nkF32 w, nkF32 h);  // Set the viewport rect to use for rendering.
GLOBAL void imm_set_projection  (nkMat4 projection);                   // Set the projection matrix to use for rendering.
GLOBAL void imm_set_view        (nkMat4 view);                         // Set the view matrix to use for rendering.
GLOBAL void imm_set_model       (nkMat4 model);                        // Set the model matrix to use for rendering.
GLOBAL void imm_set_depth_read  (nkBool enable);                       // Set whether rendering should read depth or not.
GLOBAL void imm_set_depth_write (nkBool enable);                       // Set whether rendering should write depth or not.
// =============================================================================

// State Getters ===============================================================
GLOBAL Texture imm_get_color_target(void);            // Get the color render target currently in use.
GLOBAL Texture imm_get_depth_target(void);            // Get the depth render target currently in use.
GLOBAL Shader  imm_get_shader      (void);            // Get the shader currently in use.
GLOBAL Sampler imm_get_sampler     (nkS32 slot = 0);  // Get the sampler currently in use.
GLOBAL Texture imm_get_texture     (nkS32 slot = 0);  // Get the texture currently in use.
GLOBAL fRect   imm_get_viewport    (void);            // Get the viewport currently in use.
GLOBAL nkMat4  imm_get_projection  (void);            // Get the projection matrix currently in use.
GLOBAL nkMat4  imm_get_view        (void);            // Get the view matrix currently in use.
GLOBAL nkMat4  imm_get_model       (void);            // Get the model matrix currently in use.
GLOBAL nkBool  imm_get_depth_read  (void);            // Get the current depth read status.
GLOBAL nkBool  imm_get_depth_write (void);            // Get the current depth write status.
GLOBAL Sampler imm_get_def_sampler (ImmSampler samp); // Imm defines some common samplers for systems to use, you can get them from here.
// =============================================================================

// Polygon Drawing =============================================================
// A flexible immediate mode drawing system that allows for specifying a number
// of different built-in vertex data types, as well as custom user data. Coupled
// with binding custom shaders and textures, this system can do quite a bit.
GLOBAL void imm_begin    (DrawMode draw_mode, nkBool should_clear = NK_FALSE);      // Begin a draw operation with a specifical draw mode.
GLOBAL void imm_end      (void);                                                    // End a draw operation and render to the current target.
GLOBAL void imm_position (nkF32 x, nkF32 y,        nkF32 z = 0.0f, nkF32 w = 1.0f); // Add a vertex position to the current draw data.
GLOBAL void imm_normal   (nkF32 x, nkF32 y,        nkF32 z = 0.0f, nkF32 w = 0.0f); // Add a vertex normal to the current draw data.
GLOBAL void imm_color    (nkF32 x, nkF32 y,        nkF32 z,        nkF32 w = 1.0f); // Add a vertex color to the current draw data.
GLOBAL void imm_texcoord (nkF32 x, nkF32 y,        nkF32 z = 0.0f, nkF32 w = 0.0f); // Add a vertex texcoord to the current draw data.
GLOBAL void imm_userdata0(nkF32 x, nkF32 y = 0.0f, nkF32 z = 0.0f, nkF32 w = 0.0f); // Add some custom vertex data to the current draw data.
GLOBAL void imm_userdata1(nkF32 x, nkF32 y = 0.0f, nkF32 z = 0.0f, nkF32 w = 0.0f); // Add some custom vertex data to the current draw data.
GLOBAL void imm_userdata2(nkF32 x, nkF32 y = 0.0f, nkF32 z = 0.0f, nkF32 w = 0.0f); // Add some custom vertex data to the current draw data.
GLOBAL void imm_userdata3(nkF32 x, nkF32 y = 0.0f, nkF32 z = 0.0f, nkF32 w = 0.0f); // Add some custom vertex data to the current draw data.
// =============================================================================

// 2D Primitives ===============================================================
// Convenience functions for drawing different primitives without needing to
// build the vertex data yourself and do the imm_begin and imm_end calls.
GLOBAL void imm_point         (nkF32  x, nkF32  y,                     nkVec4 color); // Draw a single point.
GLOBAL void imm_line          (nkF32 x1, nkF32 y1, nkF32 x2, nkF32 y2, nkVec4 color); // Draw a single line.
GLOBAL void imm_rect_outline  (nkF32  x, nkF32  y, nkF32  w, nkF32  h, nkVec4 color); // Draw a single rect outline.
GLOBAL void imm_rect_filled   (nkF32  x, nkF32  y, nkF32  w, nkF32  h, nkVec4 color); // Draw a single rect fill.
GLOBAL void imm_circle_outline(nkF32  x, nkF32  y, nkF32  r, nkS32  n, nkVec4 color); // Draw a single circle outline.
GLOBAL void imm_circle_filled (nkF32  x, nkF32  y, nkF32  r, nkS32  n, nkVec4 color); // Draw a single circle fill.
// =============================================================================

// 2D Textures =================================================================
// Convenience functions for rendering 2D textures (e.g. sprites) with clipping,
// simple transforms, and color modulation all built-in. Also has a simple
// texture batching system that can be used for doing some basic optimizing if
// being used to render lots of data with the same texture consecutively.
struct ImmClip { nkF32 x,y,w,h; };
GLOBAL void imm_begin_texture_batch(Texture tex);                                                                                                                            // Begin a new texture batch.
GLOBAL void imm_end_texture_batch  (void);                                                                                                                                   // End a texture batch.
GLOBAL void imm_texture            (Texture tex, nkF32 x, nkF32 y,                                                  const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE); // Draw a texture.
GLOBAL void imm_texture_ex         (Texture tex, nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE); // Draw a texture with scale and rotation.
GLOBAL void imm_texture_batched    (             nkF32 x, nkF32 y,                                                  const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE); // Draw a batched texture.
GLOBAL void imm_texture_batched_ex (             nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE); // Draw a batched texture with scale and rotation.
// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
