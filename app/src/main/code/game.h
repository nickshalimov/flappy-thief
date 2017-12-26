#ifndef GAME_H
#define GAME_H

#include "game_state.h"

#include <memory>

class bundle;
class renderer;
class user_interface;
class world;

class game final
{
public:
    game(uint32_t score, const bundle& b);
    ~game();

    void integrate(int64_t dt);
    void draw(renderer* r);
    void handle_tap_down();

    uint32_t best_score() const;

private:
    float screen_width_;
    std::unique_ptr<world> world_;
    std::unique_ptr<user_interface> user_interface_;
};

#endif
