/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL NKFORCEINLINE nkBool point_vs_point  (const fPoint&  a, const fPoint&  b);
GLOBAL NKFORCEINLINE nkBool point_vs_circle (const fPoint&  p, const fCircle& c);
GLOBAL NKFORCEINLINE nkBool point_vs_rect   (const fPoint&  p, const fRect&   r);

GLOBAL NKFORCEINLINE nkBool rect_vs_point   (const fRect&   r, const fPoint&  p);
GLOBAL NKFORCEINLINE nkBool rect_vs_circle  (const fRect&   r, const fCircle& c);
GLOBAL NKFORCEINLINE nkBool rect_vs_rect    (const fRect&   a, const fRect&   b);

GLOBAL NKFORCEINLINE nkBool circle_vs_point (const fCircle& c, const fPoint&  p);
GLOBAL NKFORCEINLINE nkBool circle_vs_circle(const fCircle& a, const fCircle& b);
GLOBAL NKFORCEINLINE nkBool circle_vs_rect  (const fCircle& c, const fRect&   r);

/*////////////////////////////////////////////////////////////////////////////*/
