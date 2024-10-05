// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "AssetLibrary.hpp"

#include "tcob/core/Logger.hpp"

namespace tcob::assets {

template <typename T>
inline bucket<T>::bucket(group& groupName)
    : detail::bucket_base {T::asset_name}
    , _group {groupName}
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
inline void bucket<T>::get_asset_stats(bucket_stats& out) const
{
    for (auto& [name, ptr] : _objects) {
        auto st {ptr.second.get()->get_status()};
        out.Assets[name] = {.Status   = st,
                            .UseCount = ptr.second.get_use_count()};
        out.Statuses[st]++;
    }
}

template <typename T>
inline auto bucket<T>::get_group() -> group&
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

    _buckets[T::asset_name] = std::make_unique<bucket<T>>(*this);
}

template <typename T>
inline auto group::get_bucket() -> bucket<T>*
{
    auto it {_buckets.find(T::asset_name)};
    return it != _buckets.end() ? dynamic_cast<bucket<T>*>(it->second.get()) : nullptr;
}

template <typename T>
inline auto group::get(string const& assetName) const -> assets::asset_ptr<T>
{
    if (!has<T>(assetName)) {
        logger::Error("AssetGroup '{}': asset '{}' not found.", _name, assetName);
        return assets::asset_ptr<T> {nullptr};
    }

    return dynamic_cast<bucket<T>*>(_buckets.at(T::asset_name).get())->get(assetName);
}

template <typename T>
inline auto group::has(string const& assetName) const -> bool
{
    auto it {_buckets.find(T::asset_name)};
    if (it == _buckets.end()) { return false; }

    return dynamic_cast<bucket<T>*>(it->second.get())->has(assetName);
}

////////////////////////////////////////////////////////////

template <typename T>
inline loader<T>::loader(group& group)
    : _group {group}
{
}

template <typename T>
inline void loader<T>::unload(asset<T>& asset, bool greedy)
{
    unload_asset(asset, greedy);
    get_bucket()->unload(asset.get_name());
}

template <typename T>
inline auto loader<T>::reload(asset<T>& asset) -> bool
{
    return reload_asset(asset);
}

template <typename T>
inline auto loader<T>::reload_asset(asset<T>& /*asset*/) -> bool { return false; };

template <typename T>
inline auto loader<T>::get_group() -> group&
{
    return _group;
}

template <typename T>
inline auto loader<T>::get_bucket() -> bucket<T>*
{
    return _group.get_bucket<T>();
}

template <typename T>
inline void loader<T>::set_asset_status(asset_ptr<T> asset, status status)
{
    asset.get()->set_status(status);

    switch (status) {
    case status::Unloaded:
        break;
    case status::Created:
        break;
    case status::Loading:
        break;
    case status::Loaded:
        logger::Info("AssetLoader: group '{}' type '{}' -> asset '{}' successfully loaded",
                     get_group().get_name(), get_bucket()->get_name(), asset.get()->get_name());
        break;
    case status::Error:
        logger::Error("AssetLoader: group '{}' type '{}' -> asset '{}' loading failed",
                      get_group().get_name(), get_bucket()->get_name(), asset.get()->get_name());
        break;
    }
}

}
