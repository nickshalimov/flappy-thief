#ifndef ANIMATION_H
#define ANIMATION_H

#include "sprite.h"

#include <cstdint>
#include <vector>

class animation final
{
public:
    animation(std::vector<sprite> frames, float rate);

    void play(bool loop);
    bool advance(int64_t delta);
    const sprite& frame() const;

private:
    const std::vector<sprite> frames_;
    const float interval_;
    int64_t time_;
    bool loop_;
};

#endif
