#ifndef MAT3_H
#define MAT3_H

#include "types.h"
#include <cmath>

inline constexpr mat3 mat3_translation(float tx, float ty)
{
    return mat3 { {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        tx, ty, 1.0f
    } };
}

inline constexpr mat3 mat3_scaling(float sx, float sy)
{
    return mat3 { {
        sx, 0.0f, 0.0f,
        0.0f, sy, 0.0f,
        0.0f, 0.0f, 1.0f
    } };
}

inline mat3 mat3_rotation(float angle)
{
    const float rad = static_cast<float>(M_PI * angle / 180.0f);
    const float cosa = cosf(rad);
    const float sina = sinf(rad);
    return mat3 { {
        cosa, -sina, 0.0f,
        sina, cosa, 0.0f,
        0.0f, 0.0f, 1.0f
    } };
}

inline constexpr mat3 operator*(mat3 m1, mat3 m2)
{
    return mat3 { {
        m1.m[0] * m2.m[0] + m1.m[1] * m2.m[3],
        m1.m[0] * m2.m[1] + m1.m[1] * m2.m[4],
        0.0f,
        m1.m[3] * m2.m[0] + m1.m[4] * m2.m[3],
        m1.m[3] * m2.m[1] + m1.m[4] * m2.m[4],
        0.0f,
        m1.m[6] * m2.m[0] + m1.m[7] * m2.m[3] + m2.m[6],
        m1.m[6] * m2.m[1] + m1.m[7] * m2.m[4] + m2.m[7],
        1.0f
    } };
}

inline constexpr vec2 operator*(mat3 m, vec2 v)
{
    return vec2 {
        m.m[0] * v.x + m.m[3] * v.y + m.m[6],
        m.m[1] * v.x + m.m[4] * v.y + m.m[7]
    };
}

#endif
