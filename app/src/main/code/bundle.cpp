#include "bundle.h"

#include <sstream>

namespace {

    std::istream& operator>>(std::istream& s, blend_mode& bm)
    {
        std::string mode;
        s >> mode;

        if (mode == "blend-none") { bm = blend_mode::none; }
        else if (mode == "blend-alpha") { bm = blend_mode::alpha; }
        else { throw std::runtime_error("unsupported blend mode: " + mode); }

        return s;
    }

}

std::istream& operator>>(std::istream& s, bundle& b)
{
    string_table<size_t> shaders_table, textures_table, materials_table;
    std::string type, id;

    while (s >> type >> id)
    {
        if (type == "shader")
        {
            shader_source shader;
            s >> shader.vert >> shader.frag;

            shaders_table.emplace(id, b.shaders_.size());
            b.shaders_.emplace_back(shader);
        }
        else if (type == "texture")
        {
            texture_source texture;
            s >> texture.path;

            textures_table.emplace(id, b.textures_.size());
            b.textures_.emplace_back(texture);
        }
        else if (type == "material")
        {
            material_source material;
            std::string texture_id, shader_id;

            s >> material.blend >> shader_id >> texture_id;
            material.shader = shaders_table[shader_id];
            material.texture = textures_table[texture_id];

            materials_table.emplace(id, b.materials_.size());
            b.materials_.emplace_back(material);
        }
        else if (type == "sprite")
        {
            sprite sprite;
            std::string material_id;

            s >> material_id
                >> sprite.rect.left >> sprite.rect.right
                >> sprite.rect.bottom >> sprite.rect.top
                >> sprite.origin.x >> sprite.origin.y;

            sprite.material = materials_table[material_id];

            b.sprites_table_.emplace(id, b.sprites_.size());
            b.sprites_.emplace_back(sprite);
        }
        else if (type == "sprite-array")
        {
            std::vector<size_t> set;
            std::string sprites;
            std::getline(s, sprites);
            std::stringstream line { sprites };

            std::string sprite_id;
            while (line >> sprite_id)
            {
                set.push_back(b.sprites_table_.at(sprite_id));
            }

            b.arrays_table_.emplace(id, std::move(set));
        }
        else if (type == "value")
        {
            float number;
            s >> number;
            b.values_table_.emplace(id, number);
        }
    }

    return s;
}

sprite bundle::sprite(const std::string& name) const
{
    return sprites_[sprites_table_.at(name)];
}

std::vector<sprite> bundle::sprite_array(const std::string& name) const
{
    const auto& indices = arrays_table_.at(name);
    std::vector<struct sprite> sprites;
    sprites.reserve(indices.size());
    
    for (auto i: indices) { sprites.emplace_back(sprites_[i]); }
    
    return std::move(sprites);
}

float bundle::value(const std::string& name) const
{
    return values_table_.at(name);
}
