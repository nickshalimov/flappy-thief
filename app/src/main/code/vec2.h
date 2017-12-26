#ifndef VEC2_H
#define VEC2_H

#include "types.h"

inline constexpr vec2 vec2_zero()
{
    return vec2 { 0.0f, 0.0f };
}

inline constexpr vec2 operator+(vec2 p1, vec2 p2)
{
    return vec2 { p1.x + p2.x, p1.y + p2.y };
}

inline constexpr vec2 operator-(vec2 p1, vec2 p2)
{
    return vec2 { p1.x - p2.x, p1.y - p2.y };
}

inline constexpr vec2 operator*(vec2 p, float s)
{
    return vec2 { p.x * s, p.y * s };
}

#endif
