/*////////////////////////////////////////////////////////////////////////////*/

static void init_audio_system(void);
static void quit_audio_system(void);

// Audio =======================================================================
static void   set_sound_volume(nkF32 volume); // [0-1]
static void   set_music_volume(nkF32 volume); // [0-1]
static nkF32  get_sound_volume(void);
static nkF32  get_music_volume(void);
static nkBool is_sound_on     (void);
static nkBool is_music_on     (void);
static nkBool is_music_playing(void);
// =============================================================================

// Sound =======================================================================
DECLARE_PRIVATE_TYPE(Sound);

typedef nkU32 SoundRef;

GLOBAL constexpr SoundRef INVALID_SOUND_REF = 0xFFFFFFFF;

static Sound    create_sound_from_file(const nkChar* file_name);
static Sound    create_sound_from_data(void* data, nkU64 size);
static void     free_sound            (Sound sound);
static SoundRef play_sound            (Sound sound, nkS32 loops = 0);
static SoundRef play_sound_on_channel (Sound sound, nkS32 loop, nkS32 channel);
static void     resume_sound          (SoundRef sound_ref);
static void     pause_sound           (SoundRef sound_ref);
static void     stop_sound            (SoundRef sound_ref);
static void     fade_out_sound        (SoundRef sound_ref, nkF32 seconds);
// =============================================================================

// Music =======================================================================
DECLARE_PRIVATE_TYPE(Music);

static Music create_music_from_file(const nkChar* file_name);
static Music create_music_from_data(void* data, nkU64 size);
static void  free_music            (Music music);
static void  play_music            (Music music, nkS32 loops);
static void  resume_music          (void);
static void  pause_music           (void);
static void  stop_music            (void);
// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
