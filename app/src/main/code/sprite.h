#ifndef SPRITE_H
#define SPRITE_H

#include "types.h"
#include <cstdlib>

struct sprite
{
    size_t material;
    struct rect rect;
    vec2 origin;
};

#endif
