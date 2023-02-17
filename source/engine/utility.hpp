/*////////////////////////////////////////////////////////////////////////////*/

#define GLOBAL     static
#define INTERNAL   static
#define PERSISTENT static

#define DECLARE_PRIVATE_TYPE(name) struct name##__Type; typedef struct name##__Type* name;
#define DEFINE_PRIVATE_TYPE(name) struct name##__Type
#define ALLOCATE_PRIVATE_TYPE(name) NK_CALLOC_TYPES(name##__Type, 1)

struct iPoint  { nkS32 x,y;         };
struct fPoint  { nkF32 x,y;         };
struct iLine   { nkS32 x1,y1,x2,y2; };
struct fLine   { nkF32 x1,y1,x2,y2; };
struct iCircle { nkS32 x,y,r;       };
struct fCircle { nkF32 x,y,r;       };
struct iRect   { nkS32 x,y,w,h;     };
struct fRect   { nkF32 x,y,w,h;     };

// String helpers.
GLOBAL wchar_t* convert_string_to_wide(const nkChar* str);
GLOBAL nkString format_string         (const nkChar* fmt, ...);
GLOBAL nkString format_string_v       (const nkChar* fmt, va_list args);

// File name/path helpers.
GLOBAL void     potentially_append_slash(nkChar* file_path, nkU64 size);
GLOBAL nkString potentially_append_slash(const nkChar* file_path);

// Random number generation.
GLOBAL void   rng_seed(nkU32 seed);
GLOBAL nkS32  rng_s32 (void);
GLOBAL nkF32  rng_f32 (void);
GLOBAL nkVec2 rng_v2  (void);
GLOBAL nkVec3 rng_v3  (void);
GLOBAL nkVec4 rng_v4  (void);
GLOBAL nkS32  rng_s32 (nkS32  min, nkS32  max);
GLOBAL nkF32  rng_f32 (nkF32  min, nkF32  max);
GLOBAL nkVec2 rng_v2  (nkVec2 min, nkVec2 max);
GLOBAL nkVec3 rng_v3  (nkVec3 min, nkVec3 max);
GLOBAL nkVec4 rng_v4  (nkVec4 min, nkVec4 max);

// String parsing helpers.
GLOBAL nkChar* str_eat_space(nkChar** str);
GLOBAL nkChar* str_get_line (nkChar** str);
GLOBAL nkChar* str_get_word (nkChar** str);
GLOBAL nkBool  str_get_bool (nkChar** str);
GLOBAL nkS32   str_get_s32  (nkChar** str, nkS32 base = 10);
GLOBAL nkF32   str_get_f32  (nkChar** str);
GLOBAL nkVec2  str_get_vec2 (nkChar** str);
GLOBAL nkVec3  str_get_vec3 (nkChar** str);
GLOBAL nkVec4  str_get_vec4 (nkChar** str);

// Miscellaneous
INTERNAL NKFORCEINLINE nkF32 distance_between_points(const nkVec2& a, const nkVec2& b);
INTERNAL NKFORCEINLINE nkF32 ease_out_elastic       (nkF32 t);

/*////////////////////////////////////////////////////////////////////////////*/
