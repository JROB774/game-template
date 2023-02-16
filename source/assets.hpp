/*////////////////////////////////////////////////////////////////////////////*/

// nkFileContent
//
template<>
nkFileContent asset_load<nkFileContent>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    nkFileContent file_content = { data, size };
    return file_content;
}
template<>
void asset_free<nkFileContent>(Asset<nkFileContent>& asset)
{
    if(!asset.from_npak) nk_free_file_content(&asset.data);
}
template<>
const nkChar* asset_path<nkFileContent>(void)
{
    return "defines/";
}

// AnimGroup
//
template<>
AnimGroup* asset_load<AnimGroup*>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_animation_group(data, size);
}
template<>
void asset_free<AnimGroup*>(Asset<AnimGroup*>& asset)
{
    free_animation_group(asset.data);
}
template<>
const nkChar* asset_path<AnimGroup*>(void)
{
    return "textures/";
}

// Sound
//
template<>
Sound asset_load<Sound>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_sound_from_data(data, size);
}
template<>
void asset_free<Sound>(Asset<Sound>& asset)
{
    return free_sound(asset.data);
}
template<>
const nkChar* asset_path<Sound>(void)
{
    return "sounds/";
}

// Music
//
template<>
Music asset_load<Music>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_music_from_data(data, size);
}
template<>
void asset_free<Music>(Asset<Music>& asset)
{
    return free_music(asset.data);
}
template<>
const nkChar* asset_path<Music>(void)
{
    return "music/";
}

// Shader
//
template<>
Shader asset_load<Shader>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    #if defined(USE_RENDERER_ADVANCED)
    ShaderDesc sd = { data, size };
    return create_shader(sd);
    #endif // USE_RENDERER_ADVANCED

    #if defined(USE_RENDERER_SIMPLE)
    return create_shader(data, size);
    #endif // USE_RENDERER_SIMPLE
}
template<>
void asset_free<Shader>(Asset<Shader>& asset)
{
    free_shader(asset.data);
}
template<>
const nkChar* asset_path<Shader>(void)
{
    return "shaders/";
}

// Texture
//
template<>
Texture asset_load<Texture>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    #if defined(USE_RENDERER_ADVANCED)
    nkS32 w,h,bpp;

    nkU8* pixels = NK_CAST(nkU8*, stbi_load_from_memory(NK_CAST(stbi_uc*, data), NK_CAST(int,size), &w,&h,&bpp, 4));
    if(!pixels) return NULL;
    NK_DEFER(stbi_image_free(pixels));

    TextureDesc desc;
    desc.format = TextureFormat_RGBA;
    desc.width  = w;
    desc.height = h;
    desc.data   = pixels;

    return create_texture(desc);
    #endif // USE_RENDERER_ADVANCED

    #if defined(USE_RENDERER_SIMPLE)
    nkS32 w,h,bpp;

    nkU8* pixels = NK_CAST(nkU8*, stbi_load_from_memory(NK_CAST(stbi_uc*, data), NK_CAST(int,size), &w,&h,&bpp, 4));
    if(!pixels) return NULL;
    NK_DEFER(stbi_image_free(pixels));

    return create_texture(w,h,4, pixels, SamplerFilter_Nearest, SamplerWrap_Clamp);
    #endif // USE_RENDERER_SIMPLE
}
template<>
void asset_free<Texture>(Asset<Texture>& asset)
{
    free_texture(asset.data);
}
template<>
const nkChar* asset_path<Texture>(void)
{
    return "textures/";
}

// BitmapFont
//
template<>
BitmapFont asset_load<BitmapFont>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_bitmap_font(data, size);
}
template<>
void asset_free<BitmapFont>(Asset<BitmapFont>& asset)
{
    free_bitmap_font(asset.data);
}
template<>
const nkChar* asset_path<BitmapFont>(void)
{
    return "fonts_bmp/";
}

// TrueTypeFont
//
template<>
TrueTypeFont asset_load<TrueTypeFont>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    // If no desc has been passed in then use the default font descriptor.
    TrueTypeFontDesc desc = (userdata) ? *NK_CAST(TrueTypeFontDesc*, userdata) : TrueTypeFontDesc();
    desc.data      = data;
    desc.size      = size;
    desc.owns_data = !from_npak;
    return create_truetype_font(desc);
}
template<>
void asset_free<TrueTypeFont>(Asset<TrueTypeFont>& asset)
{
    free_truetype_font(asset.data);
}
template<>
const nkChar* asset_path<TrueTypeFont>(void)
{
    return "fonts_ttf/";
}

/*////////////////////////////////////////////////////////////////////////////*/
