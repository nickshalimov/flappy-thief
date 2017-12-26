#include "animation.h"

animation::animation(std::vector<sprite> frames, float rate)
    : frames_(frames)
    , interval_(1000.0f / rate)
{}

void animation::play(bool loop)
{
    time_ = 0;
    loop_ = loop;
}

bool animation::advance(int64_t delta)
{
    time_ += delta;
    return loop_ || (time_ < interval_ * frames_.size());
}

const sprite& animation::frame() const
{
    return frames_[static_cast<size_t>(time_ / interval_) % frames_.size()];
}
