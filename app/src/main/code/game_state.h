#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <cstdint>

enum class game_phase { begin, play, end };

struct game_state
{
    game_phase phase = game_phase::begin;
    uint32_t best_score = 0;
    uint32_t score = 0;
    bool new_best = false;
    int64_t timer = 0;
};

#endif
