#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"

#include <cstdint>
#include <memory>

struct ANativeWindow;

class asset_loader;
class bundle;
struct sprite;

class renderer final
{
public:
    explicit renderer(ANativeWindow* w);
    ~renderer();

    void load_assets(const bundle& b, const asset_loader& loader);

    void begin_frame(float interpolation, int64_t delta);
    void end_frame();

    void set_screen_width(float width);

    void draw(const sprite& s, const mat3& matrix);
    void draw(const sprite& s, vec2 position);
    void draw(const sprite& s);

    inline float frame_interpolation() const { return frame_interpolation_; }
    inline int64_t frame_delta() const { return frame_delta_; }

private:
    class impl;
    std::unique_ptr<impl> impl_;

    float frame_interpolation_;
    int64_t frame_delta_;
};

#endif
