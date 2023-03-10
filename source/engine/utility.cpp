/*////////////////////////////////////////////////////////////////////////////*/

//
// String helpers.
//

GLOBAL wchar_t* convert_string_to_wide(const nkChar* str)
{
    nkU64 length = strlen(str);
    wchar_t* wstr = NK_MALLOC_TYPES(wchar_t, ((length+1)*4));
    if(!wstr) return NULL;
    mbstowcs(wstr, str, length);
    wstr[length] = L'\0';
    return wstr;
}

GLOBAL nkString format_string(const nkChar* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nkString str = format_string_v(fmt, args);
    va_end(args);
    return str;
}

GLOBAL nkString format_string_v(const nkChar* fmt, va_list args0)
{
    // We copy the argument list because we need to use it twice, and calling
    // vsnprintf can invalidate the original copy of the argument list.
    va_list args1;
    va_copy(args1, args0);

    nkS32 length = vsnprintf(NULL, 0, fmt, args0);

    nkString str;
    nk_string_reserve(&str, length);
    vsnprintf(str.cstr, length+1, fmt, args1);
    str.length = length;

    va_end(args1);

    return str;
}

//
// File name/path helpers.
//

GLOBAL void potentially_append_slash(nkChar* file_path, nkU64 size)
{
    nkU64 length = strlen(file_path);
    if(file_path[length-1] != '\\' && file_path[length-1] != '/')
    {
        if(length < size-1)
        {
            file_path[length  ] = '/';
            file_path[length+1] = '\0'; // Just in case the memory isn't zeroed!
        }
    }
}

GLOBAL nkString potentially_append_slash(const nkChar* file_path)
{
    nkString str = nk_string_create(file_path);
    if(!nk_string_empty(&str))
    {
        nkChar last_char = nk_string_last(&str);
        if(last_char != '\\' && last_char != '/')
        {
            nk_string_append(&str, '/');
        }
    }
    return str;
}

//
// Random number generation.
//

GLOBAL void rng_seed(nkU32 seed)
{
    srand(seed);
}

GLOBAL nkS32 rng_s32(void)
{
    return rand();
}

GLOBAL nkF32 rng_f32(void)
{
    return (NK_CAST(nkF32,rng_s32()) / NK_CAST(nkF32,RAND_MAX));
}

GLOBAL nkVec2 rng_v2(void)
{
    nkVec2 v;
    v.x = rng_f32();
    v.y = rng_f32();
    return v;
}

GLOBAL nkVec3 rng_v3(void)
{
    nkVec3 v;
    v.x = rng_f32();
    v.y = rng_f32();
    v.z = rng_f32();
    return v;
}

GLOBAL nkVec4 rng_v4(void)
{
    nkVec4 v;
    v.x = rng_f32();
    v.y = rng_f32();
    v.z = rng_f32();
    v.w = rng_f32();
    return v;
}

GLOBAL nkS32 rng_s32(nkS32 min, nkS32 max)
{
    if(min > max) NK_SWAP(nkS32,min,max);
    return ((rng_s32() % ((max+1)-min)) + min);
}

GLOBAL nkF32 rng_f32(nkF32 min, nkF32 max)
{
    return (min + NK_CAST(nkF32,rng_s32()) / (NK_CAST(nkF32,RAND_MAX) / (max-min)));
}

GLOBAL nkVec2 rng_v2(nkVec2 min, nkVec2 max)
{
    nkVec2 v;
    v.x = rng_f32(min.x,max.x);
    v.y = rng_f32(min.y,max.y);
    return v;
}

GLOBAL nkVec3 rng_v3(nkVec3 min, nkVec3 max)
{
    nkVec3 v;
    v.x = rng_f32(min.x,max.x);
    v.y = rng_f32(min.y,max.y);
    v.z = rng_f32(min.z,max.z);
    return v;
}

GLOBAL nkVec4 rng_v4(nkVec4 min, nkVec4 max)
{
    nkVec4 v;
    v.x = rng_f32(min.x,max.x);
    v.y = rng_f32(min.y,max.y);
    v.z = rng_f32(min.z,max.z);
    v.w = rng_f32(min.w,max.w);
    return v;
}

//
// String parsing helpers.
//

GLOBAL nkChar* str_eat_space(nkChar** str)
{
    NK_ASSERT(str);
    while(isspace(**str))
        (*str)++;
    return (*str);
}

GLOBAL nkChar* str_get_line(nkChar** str)
{
    NK_ASSERT(str);
    PERSISTENT nkChar linebuf[1024]; // @Improve: Dynamically grow as needed...
    str_eat_space(str);
    nkU32 index = 0;
    nkChar* s = *str;
    while(*s != '\n' && *s != '\r' && *s != '\0')
    {
        NK_ASSERT(index < NK_ARRAY_SIZE(linebuf));
        if(*s != '\r' && *s != '\n')
            linebuf[index++] = *s;
        s++;
        (*str)++;
    }
    linebuf[index] = '\0';
    return linebuf;
}

GLOBAL nkChar* str_get_word(nkChar** str)
{
    NK_ASSERT(str);
    PERSISTENT nkChar wordbuf[1024]; // @Improve: Dynamically grow as needed...
    str_eat_space(str);
    nkU32 index = 0;
    nkChar* s = *str;
    while(!isspace(*s) && *s != '\0')
    {
        NK_ASSERT(index < NK_ARRAY_SIZE(wordbuf));
        wordbuf[index++] = *s++;
        (*str)++;
    }
    wordbuf[index] = '\0';
    return wordbuf;
}

GLOBAL nkBool str_get_bool(nkChar** str)
{
    NK_ASSERT(str);
    nkChar* s = str_get_word(str);
    if(strcmp(s, "false") == 0) return NK_FALSE;
    if(strcmp(s, "true") == 0) return NK_TRUE;
    NK_ASSERT(NK_FALSE);
    return NK_FALSE;
}

GLOBAL nkS32 str_get_s32(nkChar** str, nkS32 base)
{
    NK_ASSERT(str);
    nkChar* s = str_get_word(str);
    return strtol(s, NULL, base);
}

GLOBAL nkF32 str_get_f32(nkChar** str)
{
    NK_ASSERT(str);
    nkChar* s = str_get_word(str);
    return NK_CAST(nkF32, atof(s));
}

GLOBAL nkVec2 str_get_vec2(nkChar** str)
{
    NK_ASSERT(str);
    nkVec2 v = NK_ZERO_MEM;
    v.x = str_get_f32(str);
    v.y = str_get_f32(str);
    return v;
}

GLOBAL nkVec3 str_get_vec3(nkChar** str)
{
    NK_ASSERT(str);
    nkVec3 v = NK_ZERO_MEM;
    v.x = str_get_f32(str);
    v.y = str_get_f32(str);
    v.z = str_get_f32(str);
    return v;
}

GLOBAL nkVec4 str_get_vec4(nkChar** str)
{
    NK_ASSERT(str);
    nkVec4 v = NK_ZERO_MEM;
    v.x = str_get_f32(str);
    v.y = str_get_f32(str);
    v.z = str_get_f32(str);
    v.w = str_get_f32(str);
    return v;
}

//
// Miscellaneous.
//

INTERNAL NKFORCEINLINE nkF32 distance_between_points(const nkVec2& a, const nkVec2& b)
{
    nkF32 dx = a.x - b.x;
    nkF32 dy = a.y - b.y;

    return fabsf(sqrtf(dx * dx + dy * dy));
}

INTERNAL NKFORCEINLINE nkF32 ease_out_elastic(nkF32 t)
{
    nkF32 c4 = (2.0f * NK_PI_F32) / 3.0f;

    if(t <= 0.0f) return 0.0f;
    if(t >= 1.0f) return 1.0f;

    return powf(2, -10 * t) * sinf((t * 10 - 0.75f) * c4) + 1.0f;
}

/*////////////////////////////////////////////////////////////////////////////*/
