/*////////////////////////////////////////////////////////////////////////////*/

#if defined(BUILD_NATIVE)
#include <glew.c>
#endif // BUILD_NATIVE
#if defined(BUILD_WEB)
#include <GLES3/gl3.h>
#endif // BUILD_WEB

struct OpenGLContext
{
    SDL_GLContext context;
    GLuint        vertex_array_object;
    nkBool        pass_started;
    DrawMode      current_draw_mode;
    VertexLayout* current_vertex_layout;
};

INTERNAL OpenGLContext g_ogl;

// Buffer ======================================================================

INTERNAL constexpr GLenum BUFFER_TYPE_TO_GL[] =
{
    GL_ARRAY_BUFFER,
    GL_ELEMENT_ARRAY_BUFFER,
    GL_UNIFORM_BUFFER
};

INTERNAL constexpr GLenum BUFFER_USAGE_TO_GL[] =
{
    GL_STATIC_DRAW,
    GL_DYNAMIC_DRAW,
    GL_STREAM_DRAW
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(BUFFER_TYPE_TO_GL) == BufferType_TOTAL, buffer_type_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(BUFFER_USAGE_TO_GL) == BufferUsage_TOTAL, buffer_usage_size_mismatch);

DEFINE_PRIVATE_TYPE(Buffer)
{
    GLenum usage;
    GLenum type;
    GLuint handle;
};

GLOBAL Buffer create_buffer(const BufferDesc& desc)
{
    Buffer buffer = ALLOCATE_PRIVATE_TYPE(Buffer);
    if(!buffer) fatal_error("Failed to allocate buffer!");

    buffer->usage = BUFFER_USAGE_TO_GL[desc.usage];
    buffer->type = BUFFER_TYPE_TO_GL[desc.type];

    glGenBuffers(1, &buffer->handle);

    glBindBuffer(buffer->type, buffer->handle);
    glBufferData(buffer->type, desc.bytes, desc.data, buffer->usage);
    glBindBuffer(buffer->type, GL_NONE);

    return buffer;
}

GLOBAL void free_buffer(Buffer buffer)
{
    NK_ASSERT(buffer);
    glDeleteBuffers(1, &buffer->handle);
    NK_FREE(buffer);
}

GLOBAL void update_buffer(Buffer buffer, void* data, nkU64 bytes)
{
    NK_ASSERT(buffer);

    glBindBuffer(buffer->type, buffer->handle);
    glBufferData(buffer->type, bytes, data, buffer->usage);
}

// =============================================================================

// Shader ======================================================================

DEFINE_PRIVATE_TYPE(Shader)
{
    GLenum      program;
    UniformDesc uniforms[32];
    nkU64       uniform_count;
};

INTERNAL GLuint compile_shader(void* data, nkU64 bytes, GLenum type)
{
    const nkChar* sources[2] = { NULL, NK_CAST(const nkChar*, data) };
    const GLint lengths[2] = { -1, NK_CAST(GLint, bytes) };

    #if defined(BUILD_NATIVE)
    #if defined(BUILD_DEBUG) && defined(NK_OS_WIN32)
    if(type == GL_VERTEX_SHADER) sources[0] = "#version 430\n#define VERT_SHADER 1\n";
    if(type == GL_FRAGMENT_SHADER) sources[0] = "#version 430\n#define FRAG_SHADER 1\n";
    #else
    if(type == GL_VERTEX_SHADER) sources[0] = "#version 330\n#define VERT_SHADER 1\n";
    if(type == GL_FRAGMENT_SHADER) sources[0] = "#version 330\n#define FRAG_SHADER 1\n";
    #endif // BUILD_DEBUG && NK_OS_WIN32
    #endif // BUILD_NATIVE
    #if defined(BUILD_WEB)
    if(type == GL_VERTEX_SHADER) sources[0] = "#version 300 es\n#define VERT_SHADER 1\nprecision mediump float;\n";
    if(type == GL_FRAGMENT_SHADER) sources[0] = "#version 300 es\n#define FRAG_SHADER 1\nprecision mediump float;\n";
    #endif // BUILD_WEB

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 2, sources, lengths);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        GLint info_log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
        nkChar* info_log = NK_MALLOC_TYPES(nkChar, info_log_length);
        if(info_log)
        {
            glGetShaderInfoLog(shader, info_log_length, NULL, info_log);
            printf("Failed to compile shader:\n%s\n", info_log);
            NK_FREE(info_log);
        }
        return GL_NONE;
    }

    return shader;
}

GLOBAL Shader create_shader(const ShaderDesc& desc)
{
    Shader shader = ALLOCATE_PRIVATE_TYPE(Shader);
    if(!shader) fatal_error("Failed to allocate shader!");

    GLuint vert = compile_shader(desc.data, desc.bytes, GL_VERTEX_SHADER);
    GLuint frag = compile_shader(desc.data, desc.bytes, GL_FRAGMENT_SHADER);

    shader->program = glCreateProgram();

    glAttachShader(shader->program, vert);
    glAttachShader(shader->program, frag);

    glLinkProgram(shader->program);

    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint success;
    glGetProgramiv(shader->program, GL_LINK_STATUS, &success);
    if(!success)
    {
        GLint info_log_length;
        glGetProgramiv(shader->program, GL_INFO_LOG_LENGTH, &info_log_length);
        nkChar* info_log = NK_MALLOC_TYPES(nkChar, info_log_length);
        if(info_log)
        {
            glGetProgramInfoLog(shader->program, info_log_length, NULL, info_log);
            printf("Failed to link shader:\n%s\n", info_log);
            NK_FREE(info_log);
        }
    }

    for(nkU64 i=0; i<desc.uniform_count; ++i)
        shader->uniforms[i] = desc.uniforms[i];
    shader->uniform_count = desc.uniform_count;

    return shader;
}

GLOBAL void free_shader(Shader shader)
{
    NK_ASSERT(shader);
    glDeleteProgram(shader->program);
    NK_FREE(shader);
}

// =============================================================================

// Sampler =====================================================================

INTERNAL constexpr GLenum SAMPLER_FILTER_TO_GL[] =
{
    GL_NEAREST,
    GL_LINEAR
};

INTERNAL constexpr GLenum SAMPLER_WRAP_TO_GL[] =
{
    GL_REPEAT,
    GL_CLAMP_TO_EDGE
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(SAMPLER_FILTER_TO_GL) == SamplerFilter_TOTAL, sampler_filter_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(SAMPLER_WRAP_TO_GL) == SamplerWrap_TOTAL, sampler_wrap_size_mismatch);

DEFINE_PRIVATE_TYPE(Sampler)
{
    GLuint handle;
};

GLOBAL Sampler create_sampler(const SamplerDesc& desc)
{
    Sampler sampler = ALLOCATE_PRIVATE_TYPE(Sampler);
    if(!sampler) fatal_error("Failed to allocate sampler!");

    glGenSamplers(1, &sampler->handle);

    glSamplerParameteri(sampler->handle, GL_TEXTURE_MIN_FILTER, SAMPLER_FILTER_TO_GL[desc.filter]);
    glSamplerParameteri(sampler->handle, GL_TEXTURE_MAG_FILTER, SAMPLER_FILTER_TO_GL[desc.filter]);

    glSamplerParameteri(sampler->handle, GL_TEXTURE_WRAP_S, SAMPLER_WRAP_TO_GL[desc.wrap_x]);
    glSamplerParameteri(sampler->handle, GL_TEXTURE_WRAP_T, SAMPLER_WRAP_TO_GL[desc.wrap_y]);
    glSamplerParameteri(sampler->handle, GL_TEXTURE_WRAP_R, SAMPLER_WRAP_TO_GL[desc.wrap_z]);

    return sampler;
}

GLOBAL void free_sampler(Sampler sampler)
{
    NK_ASSERT(sampler);
    glDeleteSamplers(1,&sampler->handle);
    NK_FREE(sampler);
}

// =============================================================================

// Texture =====================================================================

struct OpenGLTextureFormat
{
    GLenum internal_format;
    GLenum format;
    GLenum type;
};

INTERNAL constexpr GLenum TEXTURE_TYPE_TO_GL[] =
{
    GL_TEXTURE_2D
};

INTERNAL constexpr OpenGLTextureFormat TEXTURE_FORMAT_TO_GL[] =
{
    { GL_R8,               GL_RED,           GL_UNSIGNED_BYTE     },
    { GL_RGBA,             GL_RGBA,          GL_UNSIGNED_BYTE     },
    { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 }
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(TEXTURE_TYPE_TO_GL) == TextureType_TOTAL, texture_type_size_mistmatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(TEXTURE_FORMAT_TO_GL) == TextureFormat_TOTAL, texture_format_size_mismatch);

DEFINE_PRIVATE_TYPE(Texture)
{
    GLuint              handle;
    GLenum              type;
    OpenGLTextureFormat format;
    nkS32               width;
    nkS32               height;
};

GLOBAL Texture create_texture(const TextureDesc& desc)
{
    Texture texture = ALLOCATE_PRIVATE_TYPE(Texture);
    if(!texture) fatal_error("Failed to allocate texture!");

    glGenTextures(1, &texture->handle);

    texture->type = TEXTURE_TYPE_TO_GL[desc.type];
    texture->format = TEXTURE_FORMAT_TO_GL[desc.format];

    glBindTexture(texture->type, texture->handle);

    switch(desc.type)
    {
        case TextureType_2D:
        {
            glTexImage2D(texture->type, 0, texture->format.internal_format, desc.width,desc.height,
                0, texture->format.format, texture->format.type, desc.data);
        } break;
        default:
        {
            NK_ASSERT(NK_FALSE); // Unknown texture type!
        } break;
    }

    glBindTexture(texture->type, GL_NONE);

    texture->width = desc.width;
    texture->height = desc.height;

    return texture;
}

GLOBAL void free_texture(Texture texture)
{
    NK_ASSERT(texture);
    glDeleteTextures(1,&texture->handle);
    NK_FREE(texture);
}

GLOBAL void resize_texture(Texture texture, nkS32 width, nkS32 height)
{
    NK_ASSERT(texture);

    glBindTexture(texture->type, texture->handle);
    glTexImage2D(texture->type, 0, texture->format.internal_format, width,height,
        0, texture->format.format, texture->format.type, NULL);
    glBindTexture(texture->type, GL_NONE);

    texture->width = width;
    texture->height = height;
}

GLOBAL iPoint get_texture_size(Texture texture)
{
    NK_ASSERT(texture);
    iPoint size;
    size.x = texture->width;
    size.y = texture->height;
    return size;
}

GLOBAL nkS32 get_texture_width(Texture texture)
{
    NK_ASSERT(texture);
    return texture->width;
}

GLOBAL nkS32 get_texture_height(Texture texture)
{
    NK_ASSERT(texture);
    return texture->height;
}

// =============================================================================

// Render Pass =================================================================

DEFINE_PRIVATE_TYPE(RenderPass)
{
    GLuint         framebuffer;
    RenderPassDesc desc;
};

GLOBAL RenderPass create_render_pass(const RenderPassDesc& desc)
{
    RenderPass pass = ALLOCATE_PRIVATE_TYPE(RenderPass);
    if(!pass) fatal_error("Failed to allocate render pass!");

    pass->desc = desc;

    if(pass->desc.color_targets[0] == BACKBUFFER && pass->desc.num_color_targets == 1)
    {
        pass->framebuffer = GL_NONE;
    }
    else
    {
        glGenFramebuffers(1, &pass->framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, pass->framebuffer);

        for(nkU32 i=0; i<pass->desc.num_color_targets; ++i)
        {
            Texture target = pass->desc.color_targets[i];
            NK_ASSERT(target->type == GL_TEXTURE_2D);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, target->handle, 0);
        }
        if(pass->desc.depth_stencil_target)
        {
            Texture target = pass->desc.depth_stencil_target;
            NK_ASSERT(target->type == GL_TEXTURE_2D);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, target->handle, 0);
        }

        NK_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    }

    return pass;
}

GLOBAL void free_render_pass(RenderPass pass)
{
    NK_ASSERT(pass);
    if(pass->framebuffer != GL_NONE)
        glDeleteFramebuffers(1, &pass->framebuffer);
    NK_FREE(pass);
}

GLOBAL void begin_render_pass(RenderPass pass)
{
    NK_ASSERT(!g_ogl.pass_started); // Render pass is already in progress!

    g_ogl.pass_started = NK_TRUE;

    glBindFramebuffer(GL_FRAMEBUFFER, pass->framebuffer);

    // Clear the target(s).
    if(pass->desc.clear)
    {
        // @Todo: Implement multiple clear colors for all the color targets, depth target, and stencil target.
        GLbitfield clear_mask = GL_NONE;

        if(pass->desc.num_color_targets > 0) NK_SET_FLAGS(clear_mask, GL_COLOR_BUFFER_BIT);
        if(pass->desc.depth_stencil_target) NK_SET_FLAGS(clear_mask, GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        if(clear_mask != GL_NONE)
        {
            const nkVec4& c = pass->desc.clear_color;

            glClearColor(c.r,c.g,c.b,c.a);
            glClear(clear_mask);
        }
    }
}

GLOBAL void end_render_pass(void)
{
    NK_ASSERT(g_ogl.pass_started); // Render pass has not been started!
    g_ogl.pass_started = NK_FALSE;
}

// =============================================================================

// Render Pipeline =============================================================

struct Uniform
{
    nkS32       location;
    nkS32       binding;
    UniformType type;
};

DEFINE_PRIVATE_TYPE(RenderPipeline)
{
    RenderPipelineDesc desc;
    Uniform            uniforms[32];
    nkU64              uniform_count;
};

GLOBAL RenderPipeline create_render_pipeline(const RenderPipelineDesc& desc)
{
    RenderPipeline pipeline = ALLOCATE_PRIVATE_TYPE(RenderPipeline);
    if(!pipeline) fatal_error("Failed to allocate render pipeline!");

    pipeline->desc = desc;

    // Build a map of all the shader uniform.
    if(desc.shader)
    {
        for(nkU64 i=0; i<desc.shader->uniform_count; ++i)
        {
            const UniformDesc& udesc = desc.shader->uniforms[i];
            Uniform& u = pipeline->uniforms[i];

            u.type = udesc.type;

            if(udesc.type == UniformType_Buffer)
            {
                u.location = glGetUniformBlockIndex(desc.shader->program, udesc.name.cstr);
                u.binding = udesc.bind;
            }
            if(udesc.type == UniformType_Texture)
            {
                u.location = glGetUniformLocation(desc.shader->program, udesc.name.cstr);
                u.binding = udesc.bind;
            }

            pipeline->uniform_count++;
        }
    }

    return pipeline;
}

GLOBAL void free_render_pipeline(RenderPipeline pipeline)
{
    NK_ASSERT(pipeline);
    NK_FREE(pipeline);
}

// =============================================================================

// Renderer ====================================================================

struct OpenGLAttribType
{
    GLenum type;
    GLuint comp;
};

INTERNAL constexpr GLenum DRAW_MODE_TO_GL[] =
{
    GL_POINTS,
    GL_LINES,
    GL_LINE_STRIP,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP
};

INTERNAL constexpr GLenum DEPTH_OP_TO_GL[] =
{
    GL_NEVER,
    GL_EQUAL,
    GL_NOTEQUAL,
    GL_LESS,
    GL_LEQUAL,
    GL_GREATER,
    GL_GEQUAL,
    GL_ALWAYS
};

INTERNAL constexpr GLenum ELEMENT_TYPE_TO_GL[] =
{
    GL_UNSIGNED_SHORT,
    GL_UNSIGNED_INT
};

INTERNAL constexpr OpenGLAttribType ATTRIB_TYPE_TO_GL[] =
{
    { GL_UNSIGNED_BYTE, 4 },
    { GL_FLOAT,         1 },
    { GL_FLOAT,         2 },
    { GL_FLOAT,         3 },
    { GL_FLOAT,         4 },
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(DRAW_MODE_TO_GL) == DrawMode_TOTAL, draw_mode_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(DEPTH_OP_TO_GL) == DepthOp_TOTAL, depth_op_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(ELEMENT_TYPE_TO_GL) == ElementType_TOTAL, element_type_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(ATTRIB_TYPE_TO_GL) == AttribType_TOTAL, attrib_type_size_mismatch);

// On Windows we also enable the debug callback. We can't/don't do this on MacOS or the web build because
// they do not support OpenGL versions that have access to the debug callback unfortunately. On those
// platforms we setup both a GL3.3 and GLES3 context respectively and on Windows we setup a GL4.3 context.
#if defined(BUILD_DEBUG) && defined(NK_OS_WIN32)
INTERNAL void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userdata)
{
    NK_UNUSED(userdata);
    NK_UNUSED(length);
    NK_UNUSED(id);
    NK_UNUSED(type);
    NK_UNUSED(source);

    const nkChar* severity_string = "Info";
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW: severity_string = "Low"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: severity_string = "Medium"; break;
        case GL_DEBUG_SEVERITY_HIGH: severity_string = "High"; break;
    }

    printf("[OpenGL %s]: %s\n", severity_string, message);
}
#endif // BUILD_DEBUG && NK_OS_WIN32

GLOBAL void setup_renderer_platform(void)
{
    #if defined(BUILD_NATIVE)
    #if defined(BUILD_DEBUG) && defined(NK_OS_WIN32)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    #else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    #endif // BUILD_DEBUG && NK_OS_WIN32
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    #endif // BUILD_NATIVE
    #if defined(BUILD_WEB)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    #endif // BUILD_WEB
}

GLOBAL void init_render_system(void)
{
    printf("[OpenGL]: Initializing System\n");

    g_ogl.context = SDL_GL_CreateContext(NK_CAST(SDL_Window*, get_window()));
    if(!g_ogl.context)
    {
        fatal_error("Failed to create OpenGL context: %s", SDL_GetError());
    }

    // Enable VSync by default, if we don't get it then oh well.
    if(SDL_GL_SetSwapInterval(1) == 0)
    {
        printf("[OpenGL]: VSync Enabled!\n");
    }

    #if defined(BUILD_NATIVE)
    glewInit();
    #endif // BUILD_NATIVE

    printf("[OpenGL]: GPU Renderer   : %s\n", glGetString(GL_RENDERER));
    printf("[OpenGL]: GPU Vendor     : %s\n", glGetString(GL_VENDOR));
    printf("[OpenGL]: OpenGL Version : %s\n", glGetString(GL_VERSION));
    printf("[OpenGL]: GLSL Version   : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Enable debug logging when in debug mode for extra help/info from OpenGL.
    #if defined(BUILD_DEBUG) && defined(NK_OS_WIN32)
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_debug_callback, NULL);
    #endif // BUILD_DEBUG && NK_OS_WIN32

    // We need one Vertex Array Object in order to render with modern OpenGL.
    #if defined(BUILD_NATIVE)
    glGenVertexArrays(1, &g_ogl.vertex_array_object);
    glBindVertexArray(g_ogl.vertex_array_object);
    #endif // BUILD_NATIVE
}

GLOBAL void quit_render_system(void)
{
    #if defined(BUILD_NATIVE)
    glDeleteVertexArrays(1, &g_ogl.vertex_array_object);
    #endif // BUILD_NATIVE

    SDL_GL_DeleteContext(g_ogl.context);
}

GLOBAL void maybe_resize_backbuffer(void)
{
    // This does nothing in the OpenGL backend as the backbuffer is handled for us.
}

GLOBAL void present_renderer(void)
{
    SDL_GL_SwapWindow(NK_CAST(SDL_Window*, get_window()));
}

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    GLint   vx = NK_CAST(GLint,   x);
    GLint   vy = NK_CAST(GLint,   y);
    GLsizei vw = NK_CAST(GLsizei, w);
    GLsizei vh = NK_CAST(GLsizei, h);

    glViewport(vx,vy,vw,vh);
}

GLOBAL void begin_scissor(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    GLint   sx = NK_CAST(GLint,   x);
    GLint   sy = NK_CAST(GLint,   get_window_height()-(y+h));
    GLsizei sw = NK_CAST(GLsizei, w);
    GLsizei sh = NK_CAST(GLsizei, h);

    glScissor(sx,sy,sw,sh);
    glEnable(GL_SCISSOR_TEST);
}

GLOBAL void end_scissor(void)
{
    glDisable(GL_SCISSOR_TEST);
}

GLOBAL void bind_pipeline(RenderPipeline pipeline)
{
    NK_ASSERT(g_ogl.pass_started); // Cannot bind outside of a render pass!
    NK_ASSERT(pipeline);
    NK_ASSERT(pipeline->desc.shader);

    // Bind the current shader and setup the uniform bindings.
    glUseProgram(pipeline->desc.shader->program);

    // Setup unifrom bindings.
    for(nkU64 i = 0; i < pipeline->uniform_count; ++i)
    {
        const Uniform& u = pipeline->uniforms[i];

        if(u.type == UniformType_Buffer)
        {
            glUniformBlockBinding(pipeline->desc.shader->program, u.location, u.binding);
        }
        if(u.type == UniformType_Texture)
        {
            glUniform1i(u.location, u.binding);
        }
    }

    // Setup depth read/write.
    if(pipeline->desc.depth_read) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
    glDepthMask(pipeline->desc.depth_write);
    GLenum depth_func = DEPTH_OP_TO_GL[pipeline->desc.depth_op];
    glDepthFunc(depth_func);

    // Setup cull face mode.
    switch(pipeline->desc.cull_face)
    {
        case CullFace_None:
        {
            glDisable(GL_CULL_FACE);
        } break;
        case CullFace_Front:
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        } break;
        case CullFace_Back:
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } break;
    }

    // Setup blend mode.
    switch(pipeline->desc.blend_mode)
    {
        case BlendMode_None:
        {
            glDisable(GL_BLEND);
        } break;
        case BlendMode_Alpha:
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            glEnable(GL_BLEND);
        } break;
        case BlendMode_PremultipliedAlpha:
        {
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            glEnable(GL_BLEND);
        } break;
    }

    // Set the draw mode.
    g_ogl.current_vertex_layout = &pipeline->desc.vertex_layout;
    g_ogl.current_draw_mode = pipeline->desc.draw_mode;
}

GLOBAL void bind_buffer(Buffer buffer, nkS32 slot)
{
    NK_ASSERT(g_ogl.pass_started); // Cannot bind outside of a render pass!
    NK_ASSERT(buffer);

    if(buffer->type != GL_UNIFORM_BUFFER) glBindBuffer(buffer->type, buffer->handle);
    else glBindBufferBase(buffer->type, slot, buffer->handle);

    // Setup the vertex layout attributes for the buffer if it's a vertex layout.
    // We do this here because a buffer has to be bound before these can be set.
    if(g_ogl.current_vertex_layout && (buffer->type == GL_ARRAY_BUFFER))
    {
        for(nkS32 i=0; i<g_ogl.current_vertex_layout->attrib_count; ++i)
        {
            const VertexAttrib* attrib = &g_ogl.current_vertex_layout->attribs[i];
            if(attrib->enabled)
            {
                OpenGLAttribType type = ATTRIB_TYPE_TO_GL[attrib->type];
                glEnableVertexAttribArray(attrib->index);
                glVertexAttribPointer(attrib->index, type.comp, type.type, (type.type == GL_UNSIGNED_BYTE),
                    NK_CAST(GLsizei, g_ogl.current_vertex_layout->byte_stride), NK_CAST(const void*, attrib->byte_offset));
            }
            else
            {
                glDisableVertexAttribArray(attrib->index);
            }
        }
    }
}

GLOBAL void bind_texture(Texture texture, Sampler sampler, nkS32 unit)
{
    NK_ASSERT(g_ogl.pass_started); // Cannot bind outside of a render pass!
    NK_ASSERT(texture);

    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(texture->type, texture->handle);
    if(sampler) glBindSampler(unit, sampler->handle);
    else glBindSampler(unit, GL_NONE);
}

GLOBAL void draw_arrays(nkU64 vertex_count)
{
    NK_ASSERT(g_ogl.pass_started); // Cannot draw outside of a render pass!

    if(vertex_count == 0) return;

    GLenum mode = DRAW_MODE_TO_GL[g_ogl.current_draw_mode];
    glDrawArrays(mode, 0, NK_CAST(GLsizei,vertex_count));
}

GLOBAL void draw_elements(nkU64 element_count, ElementType element_type, nkU64 byte_offset)
{
    NK_ASSERT(g_ogl.pass_started); // Cannot draw outside of a render pass!

    if(element_count == 0) return;

    GLenum mode = DRAW_MODE_TO_GL[g_ogl.current_draw_mode];
    GLenum type = ELEMENT_TYPE_TO_GL[element_type];
    glDrawElements(mode, NK_CAST(GLsizei,element_count), type, NK_CAST(void*,byte_offset));
}

// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
