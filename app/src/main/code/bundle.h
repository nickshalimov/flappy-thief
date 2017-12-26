#ifndef BUNDLE_H
#define BUNDLE_H

#include "types.h"
#include "sprite.h"

#include <iosfwd>
#include <string>
#include <vector>
#include <unordered_map>

struct shader_source
{
    std::string vert;
    std::string frag;
};

struct texture_source
{
    std::string path;
};

struct material_source
{
    blend_mode blend;
    size_t shader;
    size_t texture;
};

template<class T>
using string_table = std::unordered_map<std::string, T>;

class bundle final
{
public:
    inline const std::vector<shader_source>& shaders() const { return shaders_; }
    inline const std::vector<texture_source>& textures() const { return textures_; }
    inline const std::vector<material_source>& materials() const { return materials_; }

    struct sprite sprite(const std::string& name) const;
    std::vector<struct sprite> sprite_array(const std::string& name) const;
    float value(const std::string& name) const;

private:
    friend std::istream& operator>>(std::istream& s, bundle& b);

    std::vector<shader_source> shaders_;
    std::vector<texture_source> textures_;
    std::vector<material_source> materials_;
    std::vector<struct sprite> sprites_;
    string_table<size_t> sprites_table_;
    string_table<std::vector<size_t>> arrays_table_;
    string_table<float> values_table_;
};

#endif
