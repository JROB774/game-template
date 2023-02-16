/*////////////////////////////////////////////////////////////////////////////*/

// Point
//
GLOBAL NKFORCEINLINE nkBool point_vs_point(const fPoint& a, const fPoint& b)
{
    return ((a.x == b.x) && (a.y == b.y));
}
GLOBAL NKFORCEINLINE nkBool point_vs_circle(const fPoint& p, const fCircle& c)
{
    return (distance_between_points({ p.x,p.y }, { c.x,c.y }) < c.r);
}
GLOBAL NKFORCEINLINE nkBool point_vs_rect(const fPoint& p, const fRect& r)
{
    return ((p.x >= r.x) && (p.x < r.x + r.w) && (p.y >= r.y) && (p.y < r.y + r.h));
}

// Rect
//
GLOBAL NKFORCEINLINE nkBool rect_vs_point(const fRect& r, const fPoint& p)
{
    return ((p.x >= r.x) && (p.x < r.x + r.w) && (p.y >= r.y) && (p.y < r.y + r.h));
}
GLOBAL NKFORCEINLINE nkBool rect_vs_circle(const fRect& r, const fCircle& c)
{
    nkVec2 nearest;
    nkVec2 ray;

    nearest.x = nk_max(r.x, nk_min(c.x, r.x+r.w));
    nearest.y = nk_max(r.y, nk_min(c.y, r.y+r.h));

    ray.x = nearest.x - c.x;
    ray.y = nearest.y - c.y;

    nkF32 overlap = (c.r - nk_length(ray));
    if(isnan(overlap))
        overlap = 0.0f;
    return (overlap > 0.0f);
}
GLOBAL NKFORCEINLINE nkBool rect_vs_rect(const fRect& a, const fRect& b)
{
    return ((a.x < b.x + b.w) && (a.x + a.w > b.x) && (a.y < b.y + b.h) && (a.y + a.h > b.y));
}

// Circle
//
GLOBAL NKFORCEINLINE nkBool circle_vs_point(const fCircle& c, const fPoint& p)
{
    return (distance_between_points({ p.x,p.y }, { c.x,c.y }) < c.r);
}
GLOBAL NKFORCEINLINE nkBool circle_vs_circle(const fCircle& a, const fCircle& b)
{
    return (nk_length(nkVec2{ a.x-b.x, a.y-b.y }) < (a.r + b.r));
}
GLOBAL NKFORCEINLINE nkBool circle_vs_rect(const fCircle& c, const fRect&  r)
{
    nkVec2 nearest;
    nkVec2 ray;

    nearest.x = nk_max(r.x, nk_min(c.x, r.x+r.w));
    nearest.y = nk_max(r.y, nk_min(c.y, r.y+r.h));

    ray.x = nearest.x - c.x;
    ray.y = nearest.y - c.y;

    nkF32 overlap = (c.r - nk_length(ray));
    if(isnan(overlap))
        overlap = 0.0f;
    return (overlap > 0.0f);
}

/*////////////////////////////////////////////////////////////////////////////*/
