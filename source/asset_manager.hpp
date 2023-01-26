/*////////////////////////////////////////////////////////////////////////////*/

// Base asset type, all assets should be specializations of Asset<T>.
struct AssetBase
{
    nkString file_name;
    nkString file_path;
    nkU64    file_size;
    nkBool   from_npak;

    virtual ~AssetBase(void) {}
};
template<typename T>
struct Asset: AssetBase
{
    T data;

    ~Asset<T>(void);
};

GLOBAL void init_asset_manager(void);
GLOBAL void quit_asset_manager(void);

template<typename T> GLOBAL T      asset_manager_load(const nkChar* name, void* userdata = NULL, const nkChar* override_path = NULL); // Use userdata to pass extra data through to asset_load.
template<typename T> GLOBAL void   asset_manager_free(const nkChar* name);
template<typename T> GLOBAL T      asset_manager_get (const nkChar* name);
template<typename T> GLOBAL nkBool asset_manager_has (const nkChar* name);

// Specialize and create these functions with your own T to implement a new asset type.
template<typename T> GLOBAL T             asset_load(void* data, nkU64 size, nkBool from_npak, void* userdata);
template<typename T> GLOBAL void          asset_free(Asset<T>& asset);
template<typename T> GLOBAL const nkChar* asset_path(void);

template<typename T>
Asset<T>::~Asset(void)
{
    asset_free<T>(*this);
}

/*////////////////////////////////////////////////////////////////////////////*/

// Assets can be defined anywhere but we are just putting all of them here for
// now so they are in one convenient location. Maybe we'll pull them out later?

// Shader ======================================================================
template<>
Shader asset_load<Shader>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    ShaderDesc sd = { data, size };
    return create_shader(sd);
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
// =============================================================================

// Texture =====================================================================
template<>
Texture asset_load<Texture>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
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
// =============================================================================

// Sound =======================================================================
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
// =============================================================================

// Music =======================================================================
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
// =============================================================================

// TrueType Font ===============================================================
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
// =============================================================================

/*
// Bitmap Font =================================================================
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
// =============================================================================
*/

// Data ========================================================================
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
// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
