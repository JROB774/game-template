/*////////////////////////////////////////////////////////////////////////////*/

static GLuint g_vao;

static void init_render_system(void)
{
    // Load OpenGL functions in a native build.
    #if defined(BUILD_NATIVE)
    glewInit();
    #endif // BUILD_NATIVE

    printf("GPU Renderer   : %s\n", glGetString(GL_RENDERER));
    printf("GPU Vendor     : %s\n", glGetString(GL_VENDOR));
    printf("OpenGL Version : %s\n", glGetString(GL_VERSION));
    printf("GLSL Version   : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // We need one Vertex Attribute Object in order to render with modern OpenGL.
    #if defined(BUILD_NATIVE)
    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);
    #endif // BUILD_NATIVE
}

static void quit_render_system(void)
{
    #if defined(BUILD_NATIVE)
    glDeleteVertexArrays(1, &g_vao);
    #endif // BUILD_NATIVE
}

static void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    GLint   vx = NK_CAST(GLint,   x);
    GLint   vy = NK_CAST(GLint,   y);
    GLsizei vw = NK_CAST(GLsizei, w);
    GLsizei vh = NK_CAST(GLsizei, h);

    glViewport(vx,vy,vw,vh);
}

static void set_blend_mode(BlendMode blend_mode)
{
    switch(blend_mode)
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
}

static void clear_screen(nkF32 r, nkF32 g, nkF32 b, nkF32 a)
{
    glClearColor(r,g,b,a);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void clear_screen(nkVec3 color)
{
    glClearColor(color.r,color.g,color.b,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void clear_screen(nkVec4 color)
{
    glClearColor(color.r,color.g,color.b,color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

// VertexBuffer ================================================================

struct VertexAttrib
{
    nkBool     enabled;
    AttribType type;
    nkU32      components;
    nkU64      byte_offset;
};

DEFINE_PRIVATE_TYPE(VertexBuffer)
{
    nkU64        byte_stride;
    GLuint       handle;
    VertexAttrib attribs[16];
};

static VertexBuffer vertex_buffer_create(void)
{
    VertexBuffer vbuf = ALLOCATE_PRIVATE_TYPE(VertexBuffer);
    if(!vbuf)
        fatal_error("Failed to allocate vertex buffer!");
    glGenBuffers(1, &vbuf->handle);
    memset(vbuf->attribs, 0, sizeof(vbuf->attribs));
    return vbuf;
}

static void vertex_buffer_destroy(VertexBuffer vbuf)
{
    if(!vbuf) return;
    glDeleteBuffers(1, &vbuf->handle);
    NK_FREE(vbuf);
}

static void vertex_buffer_set_stride(VertexBuffer vbuf, nkU64 byte_stride)
{
    NK_ASSERT(vbuf);

    vbuf->byte_stride = byte_stride;
}

static void vertex_buffer_enable_attrib(VertexBuffer vbuf, nkU32 index, AttribType type, nkU32 comps, nkU64 byte_offset)
{
    NK_ASSERT(vbuf);
    NK_ASSERT(index < NK_ARRAY_SIZE(vbuf->attribs));

    vbuf->attribs[index].enabled = NK_TRUE;
    vbuf->attribs[index].type = type;
    vbuf->attribs[index].components = comps;
    vbuf->attribs[index].byte_offset = byte_offset;
}

static void vertex_buffer_disable_attrib(VertexBuffer vbuf, nkU32 index)
{
    NK_ASSERT(vbuf);
    NK_ASSERT(index < NK_ARRAY_SIZE(vbuf->attribs));

    vbuf->attribs[index].enabled = NK_FALSE;
}

static void vertex_buffer_update(VertexBuffer vbuf, void* data, nkU64 bytes, BufferType type)
{
    NK_ASSERT(vbuf);

    GLenum gl_type;
    switch(type)
    {
        case BufferType_Static: gl_type = GL_STATIC_DRAW; break;
        case BufferType_Dynamic: gl_type = GL_DYNAMIC_DRAW; break;
        case BufferType_Stream: gl_type = GL_STREAM_DRAW; break;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbuf->handle);
    glBufferData(GL_ARRAY_BUFFER, bytes, data, gl_type);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
}

static void vertex_buffer_draw(VertexBuffer vbuf, DrawMode draw_mode, nkU64 vert_count)
{
    NK_ASSERT(vbuf);

    if(!vert_count) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbuf->handle);

    // Map the primitive type to the appropriate GL enum.
    GLenum primitive;
    switch(draw_mode)
    {
        case DrawMode_Points: primitive = GL_POINTS; break;
        case DrawMode_LineStrip: primitive = GL_LINE_STRIP; break;
        case DrawMode_LineLoop: primitive = GL_LINE_LOOP; break;
        case DrawMode_Lines: primitive = GL_LINES; break;
        case DrawMode_TriangleStrip: primitive = GL_TRIANGLE_STRIP; break;
        case DrawMode_TriangleFan: primitive = GL_TRIANGLE_FAN; break;
        case DrawMode_Triangles: primitive = GL_TRIANGLES; break;
    }

    // Setup the attributes for the buffer.
    for(nkU64 i=0, n=NK_ARRAY_SIZE(vbuf->attribs); i<n; ++i)
    {
        VertexAttrib* attrib = &vbuf->attribs[i];
        if(attrib->enabled)
        {
            GLenum attrib_type;
            switch(attrib->type)
            {
                case AttribType_SignedByte: attrib_type = GL_BYTE; break;
                case AttribType_UnsignedByte: attrib_type = GL_UNSIGNED_BYTE; break;
                case AttribType_SignedInt: attrib_type = GL_INT; break;
                case AttribType_UnsignedInt: attrib_type = GL_UNSIGNED_INT; break;
                case AttribType_Float: attrib_type = GL_FLOAT; break;
            }

            glVertexAttribPointer(NK_CAST(GLuint, i), attrib->components, attrib_type, GL_FALSE,
                NK_CAST(GLsizei, vbuf->byte_stride), NK_CAST(void*,attrib->byte_offset));
            glEnableVertexAttribArray(NK_CAST(GLuint, i));
        }
    }

    // Draw the buffer data using the provided primitive type.
    glDrawArrays(primitive, 0, NK_CAST(GLsizei,vert_count));

    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
}

// =============================================================================

// Shader ======================================================================

DEFINE_PRIVATE_TYPE(Shader)
{
    GLuint program;
};

static GLuint shader_compile(const nkChar* source, GLint bytes, GLenum type)
{
    const nkChar* sources[2] = { NULL, source };
    const GLint lengths[2] = { -1, bytes };

    #if defined(BUILD_NATIVE)
    if(type == GL_VERTEX_SHADER) sources[0] = "#version 330\n#define VERT_SHADER 1\n";
    if(type == GL_FRAGMENT_SHADER) sources[0] = "#version 330\n#define FRAG_SHADER 1\n";
    #endif // BUILD_NATIVE

    #if defined(BUILD_WEB)
    if(type == GL_VERTEX_SHADER) sources[0] = "#version 300 es\n#define VERT_SHADER 1\n";
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

static Shader shader_create(void* data, nkU64 bytes)
{
    Shader shader = ALLOCATE_PRIVATE_TYPE(Shader);
    if(!shader)
        fatal_error("Failed to allocate shader!");

    GLuint vert = shader_compile(NK_CAST(const nkChar*, data), NK_CAST(GLint, bytes), GL_VERTEX_SHADER);
    GLuint frag = shader_compile(NK_CAST(const nkChar*, data), NK_CAST(GLint, bytes), GL_FRAGMENT_SHADER);

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

    return shader;
}

static void shader_destroy(Shader shader)
{
    if(!shader) return;
    glDeleteProgram(shader->program);
    NK_FREE(shader);
}

static void shader_bind(Shader shader)
{
    if(!shader) glUseProgram(GL_NONE);
    else glUseProgram(shader->program);
}

static void shader_set_bool(Shader shader, const nkChar* name, nkBool val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("No shader uniform found: %s\n", name);
    glUniform1i(location, NK_CAST(nkS32, val));
}

static void shader_set_int(Shader shader, const nkChar* name, nkS32 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("No shader uniform found: %s\n", name);
    glUniform1i(location, val);
}

static void shader_set_float(Shader shader, const nkChar* name, nkF32 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("No shader uniform found: %s\n", name);
    glUniform1f(location, val);
}

static void shader_set_vec2(Shader shader, const nkChar* name, nkVec2 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("No shader uniform found: %s\n", name);
    glUniform2fv(location, 1, val.raw);
}

static void shader_set_vec3(Shader shader, const nkChar* name, nkVec3 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("No shader uniform found: %s\n", name);
    glUniform3fv(location, 1, val.raw);
}

static void shader_set_vec4(Shader shader, const nkChar* name, nkVec4 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("No shader uniform found: %s\n", name);
    glUniform4fv(location, 1, val.raw);
}

static void shader_set_mat2(Shader shader, const nkChar* name, nkMat2 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("No shader uniform found: %s\n", name);
    glUniformMatrix2fv(location, 1, GL_FALSE, val.raw);
}

static void shader_set_mat3(Shader shader, const nkChar* name, nkMat3 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("No shader uniform found: %s\n", name);
    glUniformMatrix3fv(location, 1, GL_FALSE, val.raw);
}

static void shader_set_mat4(Shader shader, const nkChar* name, nkMat4 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("No shader uniform found: %s\n", name);
    glUniformMatrix4fv(location, 1, GL_FALSE, val.raw);
}

// =============================================================================

// Texture =====================================================================

DEFINE_PRIVATE_TYPE(Texture)
{
    GLuint handle;
    nkVec2 size;
};

static GLenum sampler_filter_to_gl(SamplerFilter filter)
{
    switch(filter)
    {
        case SamplerFilter_Nearest: return GL_NEAREST;
        case SamplerFilter_Linear: return GL_LINEAR;
        case SamplerFilter_NearestWithNearestMips: return GL_NEAREST_MIPMAP_NEAREST;
        case SamplerFilter_LinearWithNearestMips: return GL_LINEAR_MIPMAP_NEAREST;
        case SamplerFilter_NearestWithLinearMips: return GL_NEAREST_MIPMAP_LINEAR;
        case SamplerFilter_LinearWithLinearMips: return GL_LINEAR_MIPMAP_LINEAR;
        default:
        {
            // Unsupported texture filter.
            NK_ASSERT(NK_FALSE);
        } break;
    }
    return GL_NONE;
}

static GLenum sampler_wrap_to_gl(SamplerWrap wrap)
{
    switch(wrap)
    {
        case SamplerWrap_Repeat: return GL_REPEAT;
        case SamplerWrap_Clamp: return GL_CLAMP_TO_EDGE;
        default:
        {
            // Unsupported texture wrap.
            NK_ASSERT(NK_FALSE);
        } break;
    }
    return GL_NONE;
}

static GLenum bpp_to_gl_format(nkS32 bpp)
{
    switch(bpp)
    {
        #if defined(BUILD_NATIVE)
        case 1: return GL_RED;
        case 2: return GL_RG;
        #endif // BUILD_NATIVE

        #if defined(BUILD_WEB)
        case 1: return GL_LUMINANCE;
        case 2: return GL_LUMINANCE_ALPHA;
        #endif // BUILD_WEB

        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default:
        {
            // Unsupported BPP that has no appropriate format.
            NK_ASSERT(NK_FALSE);
        } break;
    }
    return GL_NONE;
}

static Texture texture_create(nkS32 w, nkS32 h, nkS32 bpp, void* data, SamplerFilter filter, SamplerWrap wrap)
{
    Texture texture = ALLOCATE_PRIVATE_TYPE(Texture);
    if(!texture)
        fatal_error("Failed to allocate texture!");

    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &texture->handle);
    glBindTexture(GL_TEXTURE_2D, texture->handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler_filter_to_gl(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler_filter_to_gl(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler_wrap_to_gl(wrap));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler_wrap_to_gl(wrap));

    GLenum gl_format = bpp_to_gl_format(bpp);
    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, w,h, 0, gl_format, GL_UNSIGNED_BYTE, data);

    if(filter > SamplerFilter_Linear)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    texture->size.x = NK_CAST(nkF32, w);
    texture->size.y = NK_CAST(nkF32, h);

    return texture;
}

static void texture_destroy(Texture texture)
{
    if(!texture) return;
    glDeleteTextures(1, &texture->handle);
    NK_FREE(texture);
}

static void texture_bind(Texture texture, nkS32 unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    if(!texture) glBindTexture(GL_TEXTURE_2D, GL_NONE);
    else glBindTexture(GL_TEXTURE_2D, texture->handle);
}

static nkVec2 texture_get_size(Texture texture)
{
    NK_ASSERT(texture);
    return texture->size;
}

static nkF32 texture_get_width (Texture texture)
{
    NK_ASSERT(texture);
    return texture->size.x;
}

static nkF32 texture_get_height(Texture texture)
{
    NK_ASSERT(texture);
    return texture->size.y;
}

// =============================================================================

// RenderTarget ================================================================

DEFINE_PRIVATE_TYPE(RenderTarget)
{
    GLuint        handle;
    Texture       color_target;
    SamplerFilter filter;
    SamplerWrap   wrap;
};

static RenderTarget render_target_create(nkS32 w, nkS32 h, SamplerFilter filter, SamplerWrap wrap)
{
    RenderTarget target = ALLOCATE_PRIVATE_TYPE(RenderTarget);
    if(!target)
        fatal_error("Failed to allocate render target!");
    target->handle = GL_NONE;
    target->color_target = NULL;
    target->filter = filter;
    target->wrap = wrap;
    render_target_resize(target, w, h);
    return target;
}

static void render_target_destroy(RenderTarget target)
{
    if(!target) return;
    glDeleteFramebuffers(1, &target->handle);
    texture_destroy(target->color_target);
    NK_FREE(target);
}

static void render_target_resize(RenderTarget target, nkS32 w, nkS32 h)
{
    NK_ASSERT(target);

    if(w <= 0 || h <= 0) return;

    // Delete the old contents (if any).
    glDeleteFramebuffers(1, &target->handle);
    texture_destroy(target->color_target);

    glGenFramebuffers(1, &target->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, target->handle);

    target->color_target = texture_create(w,h, 4, NULL, target->filter, target->wrap);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->color_target->handle, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal_error("Failed to complete framebuffer resize!");

    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

static void render_target_bind(RenderTarget target)
{
    if(!target) glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    else glBindFramebuffer(GL_FRAMEBUFFER, target->handle);
}

// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
