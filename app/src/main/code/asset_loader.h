#ifndef ASSET_LOADER_H
#define ASSET_LOADER_H

#include <string>
#include <vector>

struct AAssetManager;

class asset_loader final
{
public:
    explicit asset_loader(AAssetManager* asset_manager);

    std::vector<uint8_t> load_bytes(const std::string& path) const;
    std::string load_string(const std::string& path) const;

private:
    AAssetManager* asset_manager_;
};

#endif
