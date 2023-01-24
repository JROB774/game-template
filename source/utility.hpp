/*////////////////////////////////////////////////////////////////////////////*/

#define GLOBAL     static
#define INTERNAL   static
#define PERSISTENT static

#define DECLARE_PRIVATE_TYPE(name) struct name##__Type; typedef struct name##__Type* name;
#define DEFINE_PRIVATE_TYPE(name) struct name##__Type
#define ALLOCATE_PRIVATE_TYPE(name) NK_MALLOC_TYPES(name##__Type, 1)

// Stack data structure.
template<typename T, nkU32 N>
struct Stack
{
    T     buffer[N] = NK_ZERO_MEM;
    nkU32 size      = 0;
};

template<typename T, nkU32 N> static NKFORCEINLINE void   stack_push (Stack<T,N>* stack, T value);
template<typename T, nkU32 N> static NKFORCEINLINE T      stack_pop  (Stack<T,N>* stack);
template<typename T, nkU32 N> static NKFORCEINLINE T      stack_peek (Stack<T,N>* stack);
template<typename T, nkU32 N> static NKFORCEINLINE nkU32  stack_size (Stack<T,N>* stack);
template<typename T, nkU32 N> static NKFORCEINLINE nkBool stack_empty(Stack<T,N>* stack);
template<typename T, nkU32 N> static NKFORCEINLINE void   stack_clear(Stack<T,N>* stack);

// String helpers.
static wchar_t* convert_string_to_wide(const nkChar* str);
static nkString format_string         (const nkChar* fmt, ...);
static nkString format_string_v       (const nkChar* fmt, va_list args);

// Collision.
static NKFORCEINLINE nkBool point_vs_rect(nkF32 px, nkF32 py, nkF32 rx, nkF32 ry, nkF32 rw, nkF32 rh);

// Random number generation.
static void   rng_seed(nkU32 seed);
static nkS32  rng_s32 (void);
static nkF32  rng_f32 (void);
static nkVec2 rng_v2  (void);
static nkVec3 rng_v3  (void);
static nkVec4 rng_v4  (void);
static nkS32  rng_s32 (nkS32  min, nkS32  max);
static nkF32  rng_f32 (nkF32  min, nkF32  max);
static nkVec2 rng_v2  (nkVec2 min, nkVec2 max);
static nkVec3 rng_v3  (nkVec3 min, nkVec3 max);
static nkVec4 rng_v4  (nkVec4 min, nkVec4 max);

// String parsing helpers.
static nkChar* str_eat_space(nkChar** str);
static nkChar* str_get_line (nkChar** str);
static nkChar* str_get_word (nkChar** str);
static nkBool  str_get_bool (nkChar** str);
static nkS32   str_get_s32  (nkChar** str, nkS32 base = 10);
static nkF32   str_get_f32  (nkChar** str);
static nkVec2  str_get_vec2 (nkChar** str);
static nkVec3  str_get_vec3 (nkChar** str);
static nkVec4  str_get_vec4 (nkChar** str);

/*////////////////////////////////////////////////////////////////////////////*/
