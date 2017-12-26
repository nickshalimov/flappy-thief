#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "animation.h"
#include "score_label.h"

#include <vector>

class bundle;
struct game_state;
class renderer;

class user_interface final
{
public:
    explicit user_interface(const bundle& b);

    void draw(renderer* r, const game_state& state);

private:
    score_label score_;
    score_label result_;
    score_label best_result_;

    animation start_anim_;
    animation repeat_anim_;

    sprite popup_;
    sprite new_best_;
};

#endif
