/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkU32 MAX_POST_PROCESS_PASSES = 32;

struct PostProcess
{
    nkStack<PostProcessEffect,MAX_POST_PROCESS_PASSES> effects;
    Texture                                            targets[2];
};

INTERNAL PostProcess g_pp;

GLOBAL void init_post_process_system(void)
{
    TextureDesc texture_desc;
    texture_desc.type   = TextureType_2D;
    texture_desc.format = TextureFormat_RGBA;
    texture_desc.width  = get_window_width();
    texture_desc.height = get_window_height();

    g_pp.targets[0] = create_texture(texture_desc);
    g_pp.targets[1] = create_texture(texture_desc);
}

GLOBAL void quit_post_process_system(void)
{
    free_texture(g_pp.targets[0]);
    free_texture(g_pp.targets[1]);
}

GLOBAL void push_post_process_effect(const PostProcessEffect& effect)
{
    nk_stack_push(&g_pp.effects, effect);
}

GLOBAL void pop_post_process_effect(void)
{
    nk_stack_pop(&g_pp.effects);
}

GLOBAL void clear_post_process_effects(void)
{
    nk_stack_clear(&g_pp.effects);
}

GLOBAL nkU64 num_post_process_effects(void)
{
    return g_pp.effects.size;
}

GLOBAL Texture perform_post_processing(Texture input)
{
    // @Improve: Too much casting back and forth, especially on window_get_xxx and texture_get_xxx...
    // These are usually always getting cast to float so we might as well start either storing them as floats
    // or providing functions that return these values as floats to at least remove the need to cast locally.

    nkF32 iw = NK_CAST(nkF32, get_texture_width(input));
    nkF32 ih = NK_CAST(nkF32, get_texture_height(input));

    Texture src = input;
    Texture dst = NULL;

    imm_reset();

    if(g_pp.effects.size > 0)
    {
        // Resize the source target if it doesn't match the input.
        nkF32 tw = NK_CAST(nkF32, get_texture_width(g_pp.targets[0]));
        nkF32 th = NK_CAST(nkF32, get_texture_height(g_pp.targets[0]));
        if(tw != iw || th != ih)
        {
            resize_texture(g_pp.targets[0], NK_CAST(nkS32,iw),NK_CAST(nkS32,ih));
            resize_texture(g_pp.targets[1], NK_CAST(nkS32,iw),NK_CAST(nkS32,ih));
        }

        imm_set_projection(nk_orthographic(0,iw,ih,0,-1,1));
        imm_set_viewport(0.0f,0.0f,iw,ih);

        // Copy the input into the first target.
        imm_set_color_target(g_pp.targets[0]);
        imm_set_texture(input);
        imm_set_shader(NULL);
        imm_begin(DrawMode_TriangleStrip);
        imm_position(0.0f,ih  ); imm_texcoord(0.0f,0.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
        imm_position(0.0f,0.0f); imm_texcoord(0.0f,1.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
        imm_position(iw,  ih  ); imm_texcoord(1.0f,0.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
        imm_position(iw,  0.0f); imm_texcoord(1.0f,1.0f); imm_color(1.0f,1.0f,1.0f,1.0f);
        imm_end();

        src = g_pp.targets[0];
        dst = g_pp.targets[1];

        for(nkU32 i=0; i<g_pp.effects.size; ++i)
        {
            const PostProcessEffect& effect = g_pp.effects.data[i];

            // Resize the output target if necessary.
            nkF32 dw = (effect.output_width == 0.0f) ? iw : effect.output_width;
            nkF32 dh = (effect.output_height == 0.0f) ? ih : effect.output_height;
            if(dw != get_texture_width(dst) || dh != get_texture_height(dst))
            {
                resize_texture(dst, NK_CAST(nkS32,dw),NK_CAST(nkS32,dh));
            }

            imm_set_projection(nk_orthographic(0,dw,dh,0,-1,1));
            imm_set_viewport(0.0f,0.0f,dw,dh);
            imm_set_color_target(dst);
            imm_set_texture(src,0);
            imm_set_sampler(effect.input_sampler,0);
            imm_set_shader(effect.effect);

            for(nkS32 j=0,n1=NK_ARRAY_SIZE(effect.textures); j<n1; ++j)
                if(effect.textures[j])
                    imm_set_texture(effect.textures[j],j+1);
            for(nkS32 j=0,n1=NK_ARRAY_SIZE(effect.samplers); j<n1; ++j)
                if(effect.samplers[j])
                    imm_set_sampler(effect.samplers[j],j+1);

            if(effect.uniforms)
            {
                imm_set_uniforms(effect.uniforms, effect.uniform_bytes, 1);
            }

            imm_begin(DrawMode_TriangleStrip);
            imm_position(0.0f,dh  ); imm_texcoord(0.0f,0.0f);
            imm_position(0.0f,0.0f); imm_texcoord(0.0f,1.0f);
            imm_position(dw,  dh  ); imm_texcoord(1.0f,0.0f);
            imm_position(dw,  0.0f); imm_texcoord(1.0f,1.0f);
            imm_end();

            NK_SWAP(Texture, src,dst);
        }
    }

    return src;
}

/*////////////////////////////////////////////////////////////////////////////*/
