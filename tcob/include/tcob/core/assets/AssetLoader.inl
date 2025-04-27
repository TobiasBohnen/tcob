// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "AssetLoader.hpp"

#include "tcob/core/Logger.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/assets/AssetGroup.hpp"
#include "tcob/core/assets/Assets.hpp"

namespace tcob::assets {

////////////////////////////////////////////////////////////

template <typename T>
inline loader<T>::loader(assets::group& group)
    : _group {group}
{
}

template <typename T>
inline auto loader<T>::group() -> assets::group&
{
    return _group;
}

template <typename T>
inline auto loader<T>::bucket() -> assets::bucket<T>*
{
    return _group.bucket<T>();
}

template <typename T>
inline void loader<T>::set_asset_status(asset_ptr<T> asset, asset_status status)
{
    asset.get()->set_status(status);

    switch (status) {
    case asset_status::Unloaded: break;
    case asset_status::Created:  break;
    case asset_status::Loading:  break;
    case asset_status::Loaded:
        logger::Info("AssetLoader: group '{}' type '{}' -> asset '{}' successfully loaded",
                     group().name(), bucket()->name(), asset.get()->name());
        break;
    case asset_status::Error:
        logger::Error("AssetLoader: group '{}' type '{}' -> asset '{}' loading failed",
                      group().name(), bucket()->name(), asset.get()->name());
        break;
    }
}

}
