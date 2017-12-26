#include "game.h"

#include "bundle.h"
#include "renderer.h"
#include "user_interface.h"
#include "world.h"

game::game(uint32_t score, const bundle& b)
    : screen_width_(b.value("screen-width"))
    , world_(new world(score, b))
    , user_interface_(new user_interface(b))
{}

game::~game() {}

void game::integrate(int64_t dt)
{
    world_->integrate(dt);
}

void game::draw(renderer* r)
{
    r->set_screen_width(screen_width_);
    world_->draw(r);
    user_interface_->draw(r, world_->state());
}

void game::handle_tap_down()
{
    world_->handle_tap();
}

uint32_t game::best_score() const
{
    return world_->state().best_score;
}
