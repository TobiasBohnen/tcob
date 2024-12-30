// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "AssetLibrary.hpp"

#include "tcob/core/Logger.hpp"

namespace tcob::assets {

template <typename T>
inline bucket<T>::bucket(group& parent)
    : detail::bucket_base {T::asset_name}
    , _group {parent}
{
}

template <typename T>
template <typename R, typename... Args>
inline auto bucket<T>::create_or_get(string const& name, loader<T>* loader, Args&&... args) -> assets::asset_ptr<T>
{
    if (!_objects.contains(name)) {
        auto obj {std::make_shared<R>(std::forward<Args>(args)...)};
        _objects[name] = {obj, assets::asset_ptr<T> {std::make_shared<asset<T>>(name, obj, loader)}};
    } else {
        auto& assetEntry {_objects[name]};
        if (!assetEntry.first) {
            assetEntry.first.reset(new R {std::forward<Args>(args)...});
            assetEntry.second.get()->reset(assetEntry.first);
        }
    }

    return _objects[name].second;
}

template <typename T>
inline auto bucket<T>::get(string const& name) const -> assets::asset_ptr<T>
{
    if (!has(name)) {
        return assets::asset_ptr<T> {nullptr};
    }
    return _objects.at(name).second;
}

template <typename T>
inline auto bucket<T>::has(string const& name) const -> bool
{
    return _objects.contains(name);
}

template <typename T>
inline void bucket<T>::destroy_all()
{
    while (!_objects.empty()) {
        destroy(_objects.begin()->first);
    }
}

template <typename T>
inline void bucket<T>::destroy(string const& name)
{
    _objects.erase(name);
}

template <typename T>
inline void bucket<T>::unload_all()
{
    for (auto& [name, _] : _objects) {
        unload(name);
    }
}

template <typename T>
inline void bucket<T>::unload(string const& name)
{
    _objects[name].first.reset();
}

template <typename T>
inline void bucket<T>::asset_stats(bucket_stats& out) const
{
    for (auto& [name, ptr] : _objects) {
        auto st {ptr.second.get()->status()};
        out.Assets[name] = {.Status   = st,
                            .UseCount = ptr.second.use_count()};
        out.Statuses[st]++;
    }
}

template <typename T>
inline auto bucket<T>::parent() -> group&
{
    return _group;
}

////////////////////////////////////////////////////////////

template <typename T>
inline void group::add_bucket()
{
    if (_buckets.contains(T::asset_name)) {
        return;
    }

    _buckets[T::asset_name] = std::make_unique<assets::bucket<T>>(*this);
}

template <typename T>
inline auto group::bucket() -> assets::bucket<T>*
{
    auto it {_buckets.find(T::asset_name)};
    return it != _buckets.end() ? dynamic_cast<assets::bucket<T>*>(it->second.get()) : nullptr;
}

template <typename T>
inline auto group::get(string const& assetName) const -> assets::asset_ptr<T>
{
    if (!has<T>(assetName)) {
        logger::Error("AssetGroup '{}': asset '{}' not found.", _name, assetName);
        return assets::asset_ptr<T> {nullptr};
    }

    return dynamic_cast<assets::bucket<T>*>(_buckets.at(T::asset_name).get())->get(assetName);
}

template <typename T>
inline auto group::has(string const& assetName) const -> bool
{
    auto it {_buckets.find(T::asset_name)};
    if (it == _buckets.end()) { return false; }

    return dynamic_cast<assets::bucket<T>*>(it->second.get())->has(assetName);
}

////////////////////////////////////////////////////////////

template <typename T>
inline loader<T>::loader(assets::group& group)
    : _group {group}
{
}

template <typename T>
inline void loader<T>::unload(asset<T>& asset, bool greedy)
{
    unload_asset(asset, greedy);
    bucket()->unload(asset.name());
}

template <typename T>
inline auto loader<T>::reload(asset<T>& asset) -> bool
{
    return reload_asset(asset);
}

template <typename T>
inline auto loader<T>::reload_asset(asset<T>& /*asset*/) -> bool { return false; };

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
    case asset_status::Created: break;
    case asset_status::Loading: break;
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
