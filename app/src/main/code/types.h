#ifndef TYPES_H
#define TYPES_H

struct vec2 { float x, y; };
struct rect { float left, right, bottom, top; };
struct mat3 { float m[9]; };

enum class blend_mode { none, alpha };

#endif
