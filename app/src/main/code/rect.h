#ifndef RECT_H
#define RECT_H

#include "types.h"

inline constexpr rect rect_zero()
{
    return rect { 0.0f, 0.0f, 0.0f, 0.0f };
}

inline constexpr vec2 rect_size(const rect& r)
{
    return vec2 { r.right - r.left, r.top - r.bottom };
}

inline constexpr rect operator+(const rect& r, vec2 p)
{
    return rect { r.left + p.x, r.right + p.x, r.bottom + p.y, r.top + p.y };
}

inline constexpr rect operator-(const rect& r, vec2 p)
{
    return rect { r.left - p.x, r.right - p.x, r.bottom - p.y, r.top - p.y };
}

#endif
