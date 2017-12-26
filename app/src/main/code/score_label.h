#ifndef SCORE_LABEL_H
#define SCORE_LABEL_H

#include "types.h"
#include "sprite.h"

#include <vector>

class renderer;

class score_label final
{
public:
    explicit score_label(std::vector<sprite> digits);

    inline void set_align(float a) { align_ = a; }
    inline void set_offset(float x, float y) { offset_ = vec2 { x, y }; }

    void draw(renderer* r, uint32_t v) const;

private:
    const std::vector<sprite> digits_;
    float align_;
    vec2 offset_;
};

#endif
