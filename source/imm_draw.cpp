/*////////////////////////////////////////////////////////////////////////////*/

#define IMM_MAX_VERTS 16384

typedef struct ImmContext
{
    DrawMode     draw_mode;
    VertexBuffer buffer;
    Shader       shader;
    Shader       bound_shader;
    Texture      bound_texture;
    nkMat4       projection;
    nkMat4       view;
    nkMat4       model;
    ImmVertex    verts[IMM_MAX_VERTS];
    nkU64        vert_count;
}
ImmContext;

static ImmContext g_imm;

static void imm_init(void)
{
    g_imm.shader = load_asset_shader("imm.shader");
    g_imm.buffer = vertex_buffer_create();
    vertex_buffer_set_stride   (g_imm.buffer, sizeof(ImmVertex));
    vertex_buffer_enable_attrib(g_imm.buffer, 0, AttribType_Float, 2, offsetof(ImmVertex, pos));
    vertex_buffer_enable_attrib(g_imm.buffer, 1, AttribType_Float, 2, offsetof(ImmVertex, tex));
    vertex_buffer_enable_attrib(g_imm.buffer, 2, AttribType_Float, 4, offsetof(ImmVertex, col));

    g_imm.projection = nk_m4_identity();
    g_imm.view       = nk_m4_identity();
    g_imm.model      = nk_m4_identity();
}

static void imm_quit(void)
{
    vertex_buffer_destroy(g_imm.buffer);
    shader_destroy(g_imm.shader);
}

static nkMat4 imm_get_projection(void)
{
    return g_imm.projection;
}

static nkMat4 imm_get_view(void)
{
    return g_imm.view;
}

static nkMat4 imm_get_model(void)
{
    return g_imm.model;
}

static void imm_set_projection(nkMat4 projection)
{
    g_imm.projection = projection;
}

static void imm_set_view(nkMat4 view)
{
    g_imm.view = view;
}

static void imm_set_model(nkMat4 model)
{
    g_imm.model = model;
}

static void imm_begin(DrawMode draw_mode, Texture tex, Shader shader)
{
    g_imm.draw_mode = draw_mode;
    g_imm.bound_texture = tex;
    g_imm.bound_shader = shader;
    g_imm.vert_count = 0;

    if(!g_imm.bound_shader)
    {
        g_imm.bound_shader = g_imm.shader;
    }
}

static void imm_end(void)
{
    texture_bind(g_imm.bound_texture, 0);
    shader_bind(g_imm.bound_shader);

    shader_set_bool(g_imm.bound_shader, "u_usetex",    (g_imm.bound_texture != NULL));
    shader_set_mat4(g_imm.bound_shader, "u_projection", g_imm.projection);
    shader_set_mat4(g_imm.bound_shader, "u_view",       g_imm.view);
    shader_set_mat4(g_imm.bound_shader, "u_model",      g_imm.model);

    vertex_buffer_update(g_imm.buffer, g_imm.verts, g_imm.vert_count * sizeof(ImmVertex), BufferType_Dynamic);
    vertex_buffer_draw(g_imm.buffer, g_imm.draw_mode, g_imm.vert_count);
}

static void imm_vertex(ImmVertex v)
{
    NK_ASSERT(g_imm.vert_count < IMM_MAX_VERTS);
    g_imm.verts[g_imm.vert_count++] = v;
}

static void imm_point(nkF32 x, nkF32 y, nkVec4 color)
{
    imm_begin(DrawMode_Points, NULL, NULL);
    imm_vertex({ { x,y }, { 0,0 }, color });
    imm_end();
}

static void imm_Line(nkF32 x1, nkF32 y1, nkF32 x2, nkF32 y2, nkVec4 color)
{
    imm_begin(DrawMode_Lines, NULL, NULL);
    imm_vertex({ { x1,y1 }, { 0,0 }, color });
    imm_vertex({ { x2,y2 }, { 1,1 }, color });
    imm_end();
}

static void imm_rect_outline(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkVec4 color)
{
    x += 0.5f;
    y += 0.5f;

    nkF32 x1 = x;
    nkF32 y1 = y;
    nkF32 x2 = x+w;
    nkF32 y2 = y+h;

    imm_begin(DrawMode_LineLoop, NULL, NULL);
    imm_vertex({ { x1,y1 }, { 0,0 }, color });
    imm_vertex({ { x2,y1 }, { 1,0 }, color });
    imm_vertex({ { x2,y2 }, { 1,1 }, color });
    imm_vertex({ { x1,y2 }, { 0,1 }, color });
    imm_end();
}

static void imm_rect_filled(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkVec4 color)
{
    nkF32 x1 = x;
    nkF32 y1 = y;
    nkF32 x2 = x+w;
    nkF32 y2 = y+h;

    imm_begin(DrawMode_TriangleStrip, NULL, NULL);
    imm_vertex({ { x1,y2 }, { 0,1 }, color });
    imm_vertex({ { x1,y1 }, { 0,0 }, color });
    imm_vertex({ { x2,y2 }, { 1,1 }, color });
    imm_vertex({ { x2,y1 }, { 1,0 }, color });
    imm_end();
}

static void imm_circle_outline(nkF32 x, nkF32 y, nkF32 r, nkS32 n, nkVec4 color)
{
    imm_begin(DrawMode_LineLoop, NULL, NULL);
    for(nkS32 i=0; i<n; ++i)
    {
        nkF32 theta = 2.0f * NK_TAU_F32 * NK_CAST(nkF32,i) / NK_CAST(nkF32,n);
        nkF32 xx = r * cosf(theta);
        nkF32 yy = r * sinf(theta);
        imm_vertex({ { xx+x,yy+y }, { 0,0 }, color });
    }
    imm_end();
}

static void imm_circle_filled(nkF32 x, nkF32 y, nkF32 r, nkS32 n, nkVec4 color)
{
    imm_begin(DrawMode_TriangleFan, NULL, NULL);
    imm_vertex({ { x,y }, { 0,0 }, color });
    for(nkS32 i=0; i<=n; ++i)
    {
        nkF32 theta = 2.0f * NK_TAU_F32 * NK_CAST(nkF32,i) / NK_CAST(nkF32,n);
        nkF32 xx = r * cosf(theta);
        nkF32 yy = r * sinf(theta);
        imm_vertex({ { xx+x,yy+y }, { 0,0 }, color });
    }
    imm_end();
}

static void imm_begin_texture_batch(Texture tex)
{
    imm_begin(DrawMode_Triangles, tex, NULL);
}

static void imm_end_texture_batch(void)
{
    imm_end();
}

static void imm_texture(Texture tex, nkF32 x, nkF32 y, const ImmRect* clip, nkVec4 color)
{
    nkF32 w = texture_get_width(tex);
    nkF32 h = texture_get_height(tex);

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 x1 = x-((s2-s1)*0.5f);
    nkF32 y1 = y-((t2-t1)*0.5f);
    nkF32 x2 = x1+(s2-s1);
    nkF32 y2 = y1+(t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    imm_begin(DrawMode_TriangleStrip, tex, NULL);
    imm_vertex({ { x1,y2 }, { s1,t2 }, color });
    imm_vertex({ { x1,y1 }, { s1,t1 }, color });
    imm_vertex({ { x2,y2 }, { s2,t2 }, color });
    imm_vertex({ { x2,y1 }, { s2,t1 }, color });
    imm_end();
}

static void imm_texture_ex(Texture tex, nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmRect* clip, nkVec4 color)
{
    nkF32 w = texture_get_width(tex);
    nkF32 h = texture_get_height(tex);

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 ox = x;
    nkF32 oy = y;

    nkF32 ax = ((anchor) ? (anchor->x*(s2-s1)) : (s2-s1)*0.5f);
    nkF32 ay = ((anchor) ? (anchor->y*(t2-t1)) : (t2-t1)*0.5f);

    x -= ax;
    y -= ay;

    nkF32 x1 = 0.0f;
    nkF32 y1 = 0.0f;
    nkF32 x2 = (s2-s1);
    nkF32 y2 = (t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    nkMat4 cached_matrix = imm_get_model();
    nkMat4 model_matrix = nk_m4_identity();

    model_matrix = nk_translate(model_matrix, {   ox,   oy, 0.0f });
    model_matrix = nk_scale    (model_matrix, {   sx,   sy, 1.0f });
    model_matrix = nk_rotate   (model_matrix, { 0.0f, 0.0f, 1.0f }, angle);
    model_matrix = nk_translate(model_matrix, {  -ox,  -oy, 0.0f });
    model_matrix = nk_translate(model_matrix, {    x,    y, 0.0f });

    imm_set_model(model_matrix);
    imm_begin(DrawMode_TriangleStrip, tex, NULL);
    imm_vertex({ { x1,y2 }, { s1,t2 }, color });
    imm_vertex({ { x1,y1 }, { s1,t1 }, color });
    imm_vertex({ { x2,y2 }, { s2,t2 }, color });
    imm_vertex({ { x2,y1 }, { s2,t1 }, color });
    imm_end();
    imm_set_model(cached_matrix);
}

static void imm_texture_batched(nkF32 x, nkF32 y, const ImmRect* clip, nkVec4 color)
{
    NK_ASSERT(g_imm.bound_texture);

    nkF32 w = texture_get_width(g_imm.bound_texture);
    nkF32 h = texture_get_height(g_imm.bound_texture);

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 x1 = x-((s2-s1)*0.5f);
    nkF32 y1 = y-((t2-t1)*0.5f);
    nkF32 x2 = x1+(s2-s1);
    nkF32 y2 = y1+(t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    imm_vertex({ { x1,y2 }, { s1,t2 }, color }); // BL
    imm_vertex({ { x1,y1 }, { s1,t1 }, color }); // TL
    imm_vertex({ { x2,y1 }, { s2,t1 }, color }); // TR
    imm_vertex({ { x2,y1 }, { s2,t1 }, color }); // TR
    imm_vertex({ { x2,y2 }, { s2,t2 }, color }); // BR
    imm_vertex({ { x1,y2 }, { s1,t2 }, color }); // BL
}

static void imm_texture_batched_ex(nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmRect* clip, nkVec4 color)
{
    NK_ASSERT(g_imm.bound_texture);

    nkF32 w = texture_get_width(g_imm.bound_texture);
    nkF32 h = texture_get_height(g_imm.bound_texture);

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 ox = x;
    nkF32 oy = y;

    nkF32 ax = ((anchor) ? (anchor->x*(s2-s1)) : (s2-s1)*0.5f);
    nkF32 ay = ((anchor) ? (anchor->y*(t2-t1)) : (t2-t1)*0.5f);

    x -= ax;
    y -= ay;

    nkF32 x1 = 0.0f;
    nkF32 y1 = 0.0f;
    nkF32 x2 = (s2-s1);
    nkF32 y2 = (t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    nkMat4 model_matrix = nk_m4_identity();

    model_matrix = nk_translate(model_matrix, {   ox,   oy, 0.0f });
    model_matrix = nk_scale    (model_matrix, {   sx,   sy, 1.0f });
    model_matrix = nk_rotate   (model_matrix, { 0.0f, 0.0f, 1.0f }, angle);
    model_matrix = nk_translate(model_matrix, {  -ox,  -oy, 0.0f });
    model_matrix = nk_translate(model_matrix, {    x,    y, 0.0f });

    nkVec4 tl = { x1,y1,0.0f,1.0f };
    nkVec4 tr = { x2,y1,0.0f,1.0f };
    nkVec4 bl = { x1,y2,0.0f,1.0f };
    nkVec4 br = { x2,y2,0.0f,1.0f };

    tl = model_matrix * tl;
    tr = model_matrix * tr;
    bl = model_matrix * bl;
    br = model_matrix * br;

    imm_vertex({ { bl.x,bl.y }, { s1,t2 }, color });
    imm_vertex({ { tl.x,tl.y }, { s1,t1 }, color });
    imm_vertex({ { tr.x,tr.y }, { s2,t1 }, color });
    imm_vertex({ { tr.x,tr.y }, { s2,t1 }, color });
    imm_vertex({ { br.x,br.y }, { s2,t2 }, color });
    imm_vertex({ { bl.x,bl.y }, { s1,t2 }, color });
}

/*////////////////////////////////////////////////////////////////////////////*/
