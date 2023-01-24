/*////////////////////////////////////////////////////////////////////////////*/

static void load_assets_npak(void);
static void free_assets_npak(void);
static void load_all_assets (void);
static void free_all_assets (void);

static Sound   load_asset_sound  (const nkChar* name);
static Music   load_asset_music  (const nkChar* name);
static Font    load_asset_font   (const nkChar* name, nkF32 px_height);
static Texture load_asset_texture(const nkChar* name, SamplerFilter filter, SamplerWrap wrap);
static Shader  load_asset_shader (const nkChar* name);

/*////////////////////////////////////////////////////////////////////////////*/
