#ifndef APP_CLOCK_H
#define APP_CLOCK_H

#include <cstdint>

class app_clock final
{
public:
    app_clock();

    int64_t now() const;
    void set_paused(bool v);
    inline bool is_paused() const { return is_paused_; }

private:
    int64_t start_;
    int64_t accumulated_;
    bool is_paused_;
};

#endif
