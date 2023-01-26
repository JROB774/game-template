/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL nkBool g_npak_loaded;
INTERNAL nkNPAK g_npak;

GLOBAL void load_assets_npak(void)
{
    #if defined(BUILD_NATIVE)
    g_npak_loaded = nk_npak_load(&g_npak, "assets.npak");
    if(!g_npak_loaded)
        printf("Failed to load assets NPAK file!\n");
    #endif // BUILD_NATIVE
}

GLOBAL void free_assets_npak(void)
{
    #if defined(BUILD_NATIVE)
    nk_npak_free(&g_npak);
    #endif // BUILD_NATIVE
}

GLOBAL void load_all_assets(void)
{
    // Nothing...
}

GLOBAL void free_all_assets(void)
{
    // Nothing...
}

/*////////////////////////////////////////////////////////////////////////////*/

// Assets will look in the NPAK file first (if in a native build) and then
// fallback to looking to a file on disk if the file could not be found.

INTERNAL constexpr const nkChar* ASSET_PATH = "../../assets/";

GLOBAL Sound load_asset_sound(const nkChar* name)
{
    nkChar buffer[1024] = NK_ZERO_MEM;

    #if defined(BUILD_NATIVE)
    if(g_npak_loaded)
    {
        strcat(buffer, "audio/sound/");
        strcat(buffer, name);
        nkU64 file_size;
        void* file_data = nk_npak_get_file_data(&g_npak, buffer, &file_size);
        if(file_data)
            return create_sound_from_data(file_data, file_size);
    }
    #endif // BUILD_NATIVE

    strcpy(buffer, get_base_path());
    strcat(buffer, ASSET_PATH);
    strcat(buffer, "audio/sound/");
    strcat(buffer, name);

    return create_sound_from_file(buffer);
}

GLOBAL Music load_asset_music(const nkChar* name)
{
    nkChar buffer[1024] = NK_ZERO_MEM;

    #if defined(BUILD_NATIVE)
    if(g_npak_loaded)
    {
        strcat(buffer, "audio/music/");
        strcat(buffer, name);
        nkU64 file_size;
        void* file_data = nk_npak_get_file_data(&g_npak, buffer, &file_size);
        if(file_data)
            return create_music_from_data(file_data, file_size);
    }
    #endif // BUILD_NATIVE

    strcpy(buffer, get_base_path());
    strcat(buffer, ASSET_PATH);
    strcat(buffer, "audio/music/");
    strcat(buffer, name);

    return create_music_from_file(buffer);
}

GLOBAL Font load_asset_font(const nkChar* name, nkF32 px_height)
{
    nkChar buffer[1024] = NK_ZERO_MEM;

    #if defined(BUILD_NATIVE)
    if(g_npak_loaded)
    {
        strcat(buffer, "fonts/");
        strcat(buffer, name);
        nkU64 file_size;
        void* file_data = nk_npak_get_file_data(&g_npak, buffer, &file_size);
        if(file_data)
            return font_create(file_data, NK_FALSE, px_height);
    }
    #endif // BUILD_NATIVE

    strcpy(buffer, get_base_path());
    strcat(buffer, ASSET_PATH);
    strcat(buffer, "fonts/");
    strcat(buffer, name);

    nkFileContent file_content;
    if(!nk_read_file_content(&file_content, buffer, nkFileReadMode_Binary))
        fatal_error("Failed to load font from file: %s", buffer);
    Font font = font_create(file_content.data, NK_TRUE, px_height);
    return font;
}

GLOBAL Texture load_asset_texture(const nkChar* name, SamplerFilter filter, SamplerWrap wrap)
{
    nkChar buffer[1024] = NK_ZERO_MEM;

    #if defined(BUILD_NATIVE)
    if(g_npak_loaded)
    {
        strcat(buffer, "textures/");
        strcat(buffer, name);
        nkU64 file_size;
        void* file_data = nk_npak_get_file_data(&g_npak, buffer, &file_size);
        if(file_data)
        {
            nkS32 w,h,bpp;
            nkU8* data = stbi_load_from_memory(NK_CAST(stbi_uc*, file_data), NK_CAST(int, file_size), &w,&h,&bpp, 4);
            if(!data)
                fatal_error("Failed to load texture from NPAK: %s", buffer);
            Texture texture = texture_create(w,h,4, data, filter, wrap);
            stbi_image_free(data);
            return texture;
        }
    }
    #endif // BUILD_NATIVE

    strcpy(buffer, get_base_path());
    strcat(buffer, ASSET_PATH);
    strcat(buffer, "textures/");
    strcat(buffer, name);

    nkS32 w,h,bpp;
    nkU8* data = stbi_load(buffer, &w,&h,&bpp, 4);
    if(!data)
        fatal_error("Failed to load texture from file: %s", buffer);
    Texture texture = texture_create(w,h,4, data, filter, wrap);
    stbi_image_free(data);
    return texture;
}

GLOBAL Shader load_asset_shader(const nkChar* name)
{
    nkChar buffer[1024] = NK_ZERO_MEM;

    #if defined(BUILD_NATIVE)
    if(g_npak_loaded)
    {
        strcat(buffer, "shaders/");
        strcat(buffer, name);
        nkU64 file_size;
        void* file_data = nk_npak_get_file_data(&g_npak, buffer, &file_size);
        if(file_data)
            return shader_create(file_data, file_size);
    }
    #endif // BUILD_NATIVE

    strcpy(buffer, get_base_path());
    strcat(buffer, ASSET_PATH);
    strcat(buffer, "shaders/");
    strcat(buffer, name);

    nkFileContent file_content = NK_ZERO_MEM;
    if(!nk_read_file_content(&file_content, buffer, nkFileReadMode_Text))
        fatal_error("Failed to load shader from file: %s", buffer);
    Shader shader = shader_create(file_content.data, file_content.size);
    nk_free_file_content(&file_content);
    return shader;
}

/*////////////////////////////////////////////////////////////////////////////*/
