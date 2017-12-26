#include "app_clock.h"

#include <ctime>

namespace {

    inline int64_t now_monotonic()
    {
        timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        return static_cast<int64_t>(time.tv_sec) * 1000 +
               static_cast<int64_t>(time.tv_nsec) / 1000000;
    }

}

app_clock::app_clock()
    : start_(now_monotonic())
    , accumulated_(0)
    , is_paused_(false)
{}

int64_t app_clock::now() const
{
    return is_paused_ ? accumulated_ : now_monotonic() + accumulated_ - start_;
}

void app_clock::set_paused(bool v)
{
    if (v == is_paused_) { return; }

    is_paused_ = v;

    if (is_paused_)
    {
        accumulated_ += now_monotonic() - start_;
    }
    else
    {
        start_ = now_monotonic();
    }
}
