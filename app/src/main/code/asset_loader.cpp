#include "asset_loader.h"

#include <android/asset_manager.h>

#include <memory>

namespace {

    using asset = std::unique_ptr<AAsset, void(*)(AAsset*)>;

    inline void close_asset(AAsset* a)
    {
        if (a != nullptr) { AAsset_close(a); }
    }

    inline asset get_asset(AAssetManager* am, const char* p)
    {
        return asset(AAssetManager_open(am, p, AASSET_MODE_BUFFER), close_asset);
    };

}

asset_loader::asset_loader(AAssetManager* asset_manager)
    : asset_manager_(asset_manager)
{}

std::vector<uint8_t> asset_loader::load_bytes(const std::string& path) const
{
    auto asset = get_asset(asset_manager_, path.c_str());
    auto buffer = (const uint8_t*)AAsset_getBuffer(asset.get());
    return std::vector<uint8_t>(buffer, buffer + AAsset_getLength(asset.get()));
}

std::string asset_loader::load_string(const std::string& path) const
{
    auto asset = get_asset(asset_manager_, path.c_str());
    auto buffer = (const char*)AAsset_getBuffer(asset.get());
    return std::string(buffer, buffer + AAsset_getLength(asset.get()));
}
