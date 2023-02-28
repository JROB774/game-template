/*////////////////////////////////////////////////////////////////////////////*/

// =============================================================================
//
// Simple implementation of Perlin Noise. This code is taken from Stefan Gustavson's
// public domain noise library (with some modifications applied for consistency).
//
// Here are his original notes regarding the noise:
//
// * This implementation is "Improved Noise" as presented by Ken Perlin at Siggraph
// * 2002. The 3D function is a direct port of his Java reference code which was once
// * publicly available on www.noisemachine.com (although I cleaned it up, made it
// * faster and made the code more readable), but the 1D, 2D and 4D functions were
// * implemented from scratch by me.
//
// And this is the link to the original repository for the noise:
//
// * https://github.com/stegu/perlin-noise
//
// =============================================================================

GLOBAL nkF32 perlin_noise_1d(nkF32 x);
GLOBAL nkF32 perlin_noise_2d(nkF32 x, nkF32 y);
GLOBAL nkF32 perlin_noise_3d(nkF32 x, nkF32 y, nkF32 z);
GLOBAL nkF32 perlin_noise_4d(nkF32 x, nkF32 y, nkF32 z, nkF32 w);

GLOBAL nkF32 periodic_perlin_noise_1d(nkF32 x, nkS32 px);
GLOBAL nkF32 periodic_perlin_noise_2d(nkF32 x, nkF32 y, nkS32 px, nkS32 py);
GLOBAL nkF32 periodic_perlin_noise_3d(nkF32 x, nkF32 y, nkF32 z, nkS32 px, nkS32 py, nkS32 pz);
GLOBAL nkF32 periodic_perlin_noise_4d(nkF32 x, nkF32 y, nkF32 z, nkF32 w, nkS32 px, nkS32 py, nkS32 pz, nkS32 pw);

// Convert the output perlin noise values from [-0.5f,+0.5f] to [0.0f,1.0f] range.
GLOBAL NKFORCEINLINE nkF32 perlin_noise_1d_normalized(nkF32 x)
{
    return (perlin_noise_1d(x) + 1.0f) * 0.5f;
}
GLOBAL NKFORCEINLINE nkF32 perlin_noise_2d_normalized(nkF32 x, nkF32 y)
{
    return (perlin_noise_2d(x, y) + 1.0f) * 0.5f;
}
GLOBAL NKFORCEINLINE nkF32 perlin_noise_3d_normalized(nkF32 x, nkF32 y, nkF32 z)
{
    return (perlin_noise_3d(x, y, z) + 1.0f) * 0.5f;
}
GLOBAL NKFORCEINLINE nkF32 perlin_noise_4d_normalized(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    return (perlin_noise_4d(x, y, z, w) + 1.0f) * 0.5f;
}

GLOBAL NKFORCEINLINE nkF32 periodic_perlin_noise_1d_normalized(nkF32 x, nkS32 px)
{
    return (periodic_perlin_noise_1d(x, px) + 1.0f) * 0.5f;
}
GLOBAL NKFORCEINLINE nkF32 periodic_perlin_noise_2d_normalized(nkF32 x, nkF32 y, nkS32 px, nkS32 py)
{
    return (periodic_perlin_noise_2d(x, y, px, py) + 1.0f) * 0.5f;
}
GLOBAL NKFORCEINLINE nkF32 periodic_perlin_noise_3d_normalized(nkF32 x, nkF32 y, nkF32 z, nkS32 px, nkS32 py, nkS32 pz)
{
    return (periodic_perlin_noise_3d(x, y, z, px, py, pz) + 1.0f) * 0.5f;
}
GLOBAL NKFORCEINLINE nkF32 periodic_perlin_noise_4d_normalized(nkF32 x, nkF32 y, nkF32 z, nkF32 w, nkS32 px, nkS32 py, nkS32 pz, nkS32 pw)
{
    return (periodic_perlin_noise_4d(x, y, z, w, px, py, pz, pw) + 1.0f) * 0.5f;
}

/*////////////////////////////////////////////////////////////////////////////*/
