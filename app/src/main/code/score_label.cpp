#include "score_label.h"

#include "rect.h"
#include "renderer.h"
#include "vec2.h"

#include <cassert>
#include <limits>

score_label::score_label(std::vector<sprite> digits)
    : digits_(digits)
    , align_(0.0f)
    , offset_(vec2 { 0.0f, 0.0f })
{
    assert(digits.size() == 10);
}

void score_label::draw(renderer* r, uint32_t v) const
{
    float width = 0.0f;
    uint32_t sprites[std::numeric_limits<uint32_t>::digits10 + 1];
    uint32_t count = 0;

    do
    {
        uint32_t d = v % 10;
        v /= 10;

        width += rect_size(digits_[d].rect).x;
        sprites[count++] = d;
    }
    while (v != 0);

    auto position = offset_ - vec2 { align_ * width, 0.0f };
    for (size_t i = count; i != 0; --i)
    {
        const sprite& s = digits_[sprites[i - 1]];
        r->draw(s, position);
        position.x += rect_size(s.rect).x;
    }
}
