#include "world.h"

#include "bundle.h"
#include "mat3.h"
#include "rect.h"
#include "vec2.h"
#include "renderer.h"

#include <algorithm>

namespace {

    const size_t NUM_SPANS = 3;
    const size_t NUM_OBSTACLES_IN_SPAN = 4;
    const size_t NUM_STROKES_IN_SPAN = 6;
    const size_t NUM_BACKS = 2;

    inline constexpr float lerp(float v1, float v2, float t)
    {
        return t * v2 + (1.0f - t) * v1;
    }

}

world::world(uint32_t best_score, const bundle& b)
    : fly_anim_(b.sprite_array("fly-anim"), b.value("fly-anim-rate"))
    , death_anim_(b.sprite_array("death-anim"), b.value("death-anim-rate"))
    , background_(b.sprite("background"))
    , stroke_sprites_(b.sprite_array("strokes"))
    , obstacles_(NUM_SPANS * NUM_OBSTACLES_IN_SPAN,
        obstacle { vec2_zero(), rect_zero(), b.sprite("ground") })
    , strokes_(NUM_SPANS * NUM_STROKES_IN_SPAN)
    , spans_(NUM_SPANS)
{
    srand((unsigned int)time(nullptr));

    state_.best_score = best_score;

    settings_.move_velocity = b.value("move-velocity");
    settings_.back_velocity = b.value("back-velocity");
    settings_.jump_velocity = b.value("jump-velocity");
    settings_.jump_angle = b.value("jump-angle");
    settings_.rotation_speed = b.value("rotation-speed");
    settings_.gravity = b.value("gravity");
    settings_.character_radius = b.value("character-radius");
    settings_.span_width = b.value("span-width");
    settings_.tube_width = b.value("tube-width");
    settings_.bound_inner = b.value("bound-inner");
    settings_.bound_outer = b.value("bound-outer");
    settings_.hole_size = b.value("hole-rect_size");
    settings_.hole_range = b.value("hole-range");

    character_.x = b.value("character-x");

    set_phase(game_phase::begin);
}

void world::integrate(int64_t dt)
{
    old_.character_y = character_.y;
    old_.world_x = world_x_;

    float sec = dt * 0.001f;

    switch (state_.phase)
    {
        case game_phase::begin:
            move_spans(sec, false);
            break;

        case game_phase::play:
            move_spans(sec, true);
            move_character(sec);

            state_.score += collect_points();

            if (has_collision())
            {
                state_.new_best = state_.score > state_.best_score;
                if (state_.new_best)
                {
                    state_.best_score = state_.score;
                }

                set_phase(game_phase::end);
            }
            break;

        case game_phase::end:
            state_.timer = std::max<int64_t>(state_.timer - dt, 0);
            break;
    }
}

void world::draw(renderer* r)
{
    const float interpolation = r->frame_interpolation();
    const float dt = r->frame_delta() * 0.001f;

    const float back_width = rect_size(background_.rect).x;

    if (state_.phase != game_phase::end)
    {
        back_x_ = fmodf(back_x_ - settings_.back_velocity * dt, back_width);
    }

    for (size_t i = 0; i < NUM_BACKS; ++i)
    {
        r->draw(background_, vec2 { back_x_ + i * back_width, 0.0f });
    }

    const float world_offset = lerp(old_.world_x, world_x_, interpolation);

    for (size_t i = 0; i < obstacles_.size(); ++i)
    {
        const float span_offset = spans_[i / NUM_OBSTACLES_IN_SPAN].offset_x * settings_.span_width;
        r->draw(obstacles_[i].view, obstacles_[i].position + vec2 { span_offset + world_offset, 0.0f });
    }

    for (size_t i = 0; i < strokes_.size(); ++i)
    {
        const float span_offset = spans_[i / NUM_STROKES_IN_SPAN].offset_x * settings_.span_width;
        r->draw(stroke_sprites_[strokes_[i].sprite_index],
             strokes_[i].matrix * mat3_translation(span_offset + world_offset, 0.0f));
    }

    if (current_anim_->advance(r->frame_delta()))
    {
        if (state_.phase == game_phase::play)
        {
            character_.angle += dt * settings_.rotation_speed;
        }

        const float char_y = lerp(old_.character_y, character_.y, interpolation);

        r->draw(current_anim_->frame(),
            mat3_rotation(character_.angle) * mat3_translation(character_.x, char_y)
        );
    }
}

void world::handle_tap()
{
    switch (state_.phase)
    {
        case game_phase::begin:
            set_phase(game_phase::play);
            break;

        case game_phase::play:
            character_.velocity = settings_.jump_velocity;
            character_.angle = settings_.jump_angle;
            break;

        case game_phase::end:
            if (state_.timer == 0)
            {
                set_phase(game_phase::begin);
            }
            break;
    }
}

void world::set_phase(game_phase phase)
{
    state_.phase = phase;

    switch (state_.phase)
    {
        case game_phase::begin:
            state_.score = 0;

            character_.y = 0.0f;
            character_.velocity = 0.0f;
            character_.angle = 0.0f;

            fly_anim_.play(true);
            current_anim_ = &fly_anim_;

            reset_spans();
            break;

        case game_phase::play:
            character_.velocity = settings_.jump_velocity;
            character_.angle = settings_.jump_angle;
            break;

        case game_phase::end:
            state_.timer = 1000;

            death_anim_.play(false);
            current_anim_ = &death_anim_;

            character_.angle = 0.0f;
            break;
    }
}

void world::move_spans(float sec, bool add_hole)
{
    world_x_ -= settings_.move_velocity * sec;

    for (size_t i = 0; i < NUM_SPANS; ++i)
    {
        span& s = spans_[i];
        const float span_offset = world_x_ + s.offset_x * settings_.span_width;
        if (span_offset > -2.0f * settings_.span_width) { continue; }

        s.offset_x += NUM_SPANS;

        if (add_hole)
        {
            s.points = 1;

            const float arb = settings_.hole_range * (rand() / (float)RAND_MAX - 0.5f);
            obstacle* obstacles = &obstacles_[i * NUM_OBSTACLES_IN_SPAN];
            obstacles[2].collider.top = settings_.bound_outer + arb - (settings_.hole_size * 0.5f);
            obstacles[3].collider.bottom = -settings_.bound_outer + arb + (settings_.hole_size  * 0.5f);
        }

        update_span_view(i);
    }
}

void world::move_character(float sec)
{
    character_.velocity += settings_.gravity * sec;
    character_.y += character_.velocity * sec;
}

uint32_t world::collect_points()
{
    const float tube_center = settings_.span_width - 0.5f * settings_.tube_width;
    uint32_t points = 0;

    for (auto& s: spans_)
    {
        if (s.points == 0) { continue; }
        const float span_offset = world_x_ + s.offset_x * settings_.span_width;
        if (character_.x > span_offset + tube_center)
        {
            points += s.points;
            s.points = 0;
        }
    }

    return points;
}

bool world::has_collision() const
{
    const vec2 c { character_.x, character_.y };
    const float sqr = settings_.character_radius * settings_.character_radius;

    for (size_t i = 0; i < obstacles_.size(); ++i)
    {
        const size_t span_index = i / NUM_OBSTACLES_IN_SPAN;
        const float span_offset = world_x_ + spans_[span_index].offset_x * settings_.span_width;

        const obstacle& o = obstacles_[i];
        const auto rect = o.collider + o.position + vec2 { span_offset, 0.0f };

        float dx = (c.x < rect.left) ? rect.left : (c.x > rect.right) ? rect.right : c.x;
        float dy = (c.y < rect.bottom) ? rect.bottom : (c.y > rect.top) ? rect.top : c.y;

        dx -= c.x;
        dy -= c.y;

        if (dx * dx + dy * dy < sqr) { return true; }
    }

    return false;
}

void world::reset_spans()
{
    world_x_ = -settings_.span_width * 2.0f;
    back_x_ = 0.0f;

    const float tube_offset = settings_.span_width - settings_.tube_width;
    const float ground_height = settings_.bound_outer - settings_.bound_inner;

    for (uint32_t i = 0; i < NUM_SPANS; ++i)
    {
        spans_[i].offset_x = i;
        spans_[i].points = 0;

        obstacle* obstacles = &obstacles_[i * NUM_OBSTACLES_IN_SPAN];

        obstacles[0].position = { 0.0f, -settings_.bound_outer };
        obstacles[0].collider = { 0.0f, tube_offset, 0.0f, ground_height };

        obstacles[1].position = { 0.0f, settings_.bound_outer };
        obstacles[1].collider = { 0.0f, tube_offset, -ground_height, 0.0f };

        obstacles[2].position = { tube_offset, -settings_.bound_outer };
        obstacles[2].collider = { 0.0f, settings_.tube_width, 0.0f, ground_height };

        obstacles[3].position = { tube_offset, settings_.bound_outer };
        obstacles[3].collider = { 0.0f, settings_.tube_width, -ground_height, 0.0f };

        update_span_view(i);
    }
}

void world::update_span_view(size_t span_index)
{
    const vec2 span_offset {
        spans_[span_index].offset_x * settings_.span_width,
        settings_.bound_outer
    };

    obstacle* obstacles = &obstacles_[span_index * NUM_OBSTACLES_IN_SPAN];
    for (size_t i = 0; i < NUM_OBSTACLES_IN_SPAN; ++i)
    {

        obstacle& o = obstacles[i];
        o.view.rect = o.collider + o.position + span_offset;
        o.view.origin.y = -o.collider.bottom;
    }

    const float tube_x = settings_.span_width - settings_.tube_width;
    const float ground = settings_.bound_outer - settings_.bound_inner;
    const float bottom = rect_size(obstacles[2].collider).y - ground;
    const float top = rect_size(obstacles[3].collider).y - ground;

    const float len[] = { 3.0f + tube_x, -bottom, settings_.tube_width, -bottom, top, top };
    const float rot[] = { 0.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f };
    const vec2 pos[] = {
        { -2.0f, -settings_.bound_inner },
        { tube_x, -settings_.bound_inner + bottom },
        { tube_x, -settings_.bound_inner + bottom },
        { settings_.span_width, -settings_.bound_inner },
        { tube_x, settings_.bound_inner - top },
        { settings_.span_width, settings_.bound_inner }
    };

    stroke* strokes = &strokes_[span_index * NUM_STROKES_IN_SPAN];
    for (size_t i = 0; i < NUM_STROKES_IN_SPAN; ++i)
    {
        size_t sprite_index = rand() % stroke_sprites_.size();
        strokes[i].sprite_index = sprite_index;
        strokes[i].matrix =
            mat3_scaling(len[i] / rect_size(stroke_sprites_[sprite_index].rect).x, 1.0f) *
            mat3_rotation(rot[i] * 90.0f) *
            mat3_translation(pos[i].x, pos[i].y);
    }
}
