// Copyright (c) 2026 Tobias Bohnen
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
inline void loader<T>::set_asset_status(asset_ptr<T> asset, asset_status status)
{
    asset.get()->Status = status;

    switch (status) {
    case asset_status::Uninitiated: break;
    case asset_status::Unloaded:    break;
    case asset_status::Prepared:    break;
    case asset_status::Loading:     break;
    case asset_status::Loaded:
        logger::Info("asset_loader: group '{}' type '{}' -> asset '{}' successfully loaded",
                     group().name(), group().bucket<T>()->name(), asset.get()->name());
        break;
    case asset_status::Error:
        logger::Error("asset_loader: group '{}' type '{}' -> asset '{}' loading failed",
                      group().name(), group().bucket<T>()->name(), asset.get()->name());
        break;
    }
}

}
