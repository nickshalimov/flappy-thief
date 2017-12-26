#include "user_interface.h"

#include "bundle.h"
#include "game_state.h"
#include "renderer.h"

user_interface::user_interface(const bundle& b)
    : score_(b.sprite_array("points-digits"))
    , result_(b.sprite_array("result-digits"))
    , best_result_(b.sprite_array("result-digits"))
    , start_anim_(b.sprite_array("start-anim"), b.value("start-anim-rate"))
    , repeat_anim_(b.sprite_array("repeat-anim"), b.value("repeat-anim-rate"))
    , popup_(b.sprite("popup"))
    , new_best_(b.sprite("new-best"))
{
    score_.set_align(b.value("points-align"));
    score_.set_offset(b.value("points-offset-x"), b.value("points-offset-y"));

    result_.set_align(b.value("result-align"));
    result_.set_offset(b.value("result-offset-x"), b.value("result-offset-y"));

    best_result_.set_align(b.value("best-align"));
    best_result_.set_offset(b.value("best-offset-x"), b.value("best-offset-y"));

    start_anim_.play(true);
    repeat_anim_.play(true);
}

void user_interface::draw(renderer* r, const game_state& state)
{
    switch (state.phase)
    {
        case game_phase::begin:
            start_anim_.advance(r->frame_delta());
            r->draw(start_anim_.frame());
            break;

        case game_phase::play:
            score_.draw(r, state.score);
            break;

        case game_phase::end:
            if (state.timer > 0)
            {
                score_.draw(r, state.score);
            }
            else
            {
                r->draw(popup_);
                if (state.new_best) { r->draw(new_best_); }

                repeat_anim_.advance(r->frame_delta());
                r->draw(repeat_anim_.frame());

                result_.draw(r, state.score);
                best_result_.draw(r, state.best_score);
            }
            break;
    }
}
