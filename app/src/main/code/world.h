#ifndef WORLD_H
#define WORLD_H

#include "animation.h"

#include "game_state.h"

class bundle;
class renderer;

class world final
{
public:
    world(uint32_t best_score, const bundle& b);

    void integrate(int64_t dt);
    void draw(renderer* r);
    void handle_tap();

    inline const game_state& state() const { return state_; }

private:
    game_state state_;

    animation fly_anim_;
    animation death_anim_;
    animation* current_anim_;

    sprite background_;
    std::vector<sprite> stroke_sprites_;

    struct {
        float move_velocity;
        float back_velocity;
        float jump_velocity;
        float jump_angle;
        float rotation_speed;
        float gravity;
        float character_radius;
        float span_width;
        float tube_width;
        float bound_inner;
        float bound_outer;
        float hole_size;
        float hole_range;
    } settings_;

    struct {
        float character_y;
        float world_x;
    } old_;

    struct {
        float x, y;
        float velocity;
        float angle;
    } character_;

    float world_x_;
    float back_x_;

    struct span
    {
        uint32_t offset_x;
        uint32_t points;
    };

    struct obstacle
    {
        vec2 position;
        rect collider;
        sprite view;
    };

    struct stroke
    {
        mat3 matrix;
        size_t sprite_index;
    };

    std::vector<obstacle> obstacles_;
    std::vector<stroke> strokes_;
    std::vector<span> spans_;

    void set_phase(game_phase phase);
    void move_spans(float sec, bool add_hole);
    void move_character(float sec);
    uint32_t collect_points();
    bool has_collision() const;
    void reset_spans();
    void update_span_view(size_t i);
};

#endif
