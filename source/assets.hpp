/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void load_assets_npak(void);
GLOBAL void free_assets_npak(void);
GLOBAL void load_all_assets (void);
GLOBAL void free_all_assets (void);

GLOBAL Sound   load_asset_sound  (const nkChar* name);
GLOBAL Music   load_asset_music  (const nkChar* name);
GLOBAL Font    load_asset_font   (const nkChar* name, nkF32 px_height);
GLOBAL Texture load_asset_texture(const nkChar* name, SamplerFilter filter, SamplerWrap wrap);
GLOBAL Shader  load_asset_shader (const nkChar* name);

/*////////////////////////////////////////////////////////////////////////////*/
