// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "AssetGroup.hpp"

#include "tcob/core/Logger.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/assets/Assets.hpp"

namespace tcob::assets {

template <typename T>
inline bucket<T>::bucket(group& parent)
    : detail::bucket_base {T::AssetName}
    , _group {parent}
{
}

template <typename T>
template <typename R, typename... Args>
inline auto bucket<T>::create(string const& name, Args&&... args) -> asset_ptr<T>
{
    if (!_objects.contains(name)) {
        auto obj {std::make_shared<R>(std::forward<Args>(args)...)};
        _objects[name] = {obj, asset_ptr<T> {std::make_shared<asset<T>>(name, obj)}};
    } else {
        auto& assetEntry {_objects[name]};
        assetEntry.first.reset(new R {std::forward<Args>(args)...});
        assetEntry.second.get()->reset(assetEntry.first);
    }

    return _objects[name].second;
}

template <typename T>
inline auto bucket<T>::get(string const& name) const -> asset_ptr<T>
{
    if (!has(name)) {
        return asset_ptr<T> {nullptr};
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
    auto& obj {_objects[name]};
    obj.first.reset();
    obj.second.get()->set_status(asset_status::Unloaded);
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
    if (_buckets.contains(typeid(T).hash_code())) {
        return;
    }

    _buckets[typeid(T).hash_code()] = std::make_unique<assets::bucket<T>>(*this);
}

template <typename T>
inline auto group::bucket() -> assets::bucket<T>*
{
    auto it {_buckets.find(typeid(T).hash_code())};
    return it != _buckets.end() ? dynamic_cast<assets::bucket<T>*>(it->second.get()) : nullptr;
}

template <typename T>
inline auto group::get(string const& assetName) const -> asset_ptr<T>
{
    if (!has<T>(assetName)) {
        logger::Error("AssetGroup '{}': asset '{}' not found.", _name, assetName);
        return asset_ptr<T> {nullptr};
    }

    return dynamic_cast<assets::bucket<T>*>(_buckets.at(typeid(T).hash_code()).get())->get(assetName);
}

template <typename T>
inline auto group::has(string const& assetName) const -> bool
{
    auto it {_buckets.find(typeid(T).hash_code())};
    if (it == _buckets.end()) { return false; }

    return dynamic_cast<assets::bucket<T>*>(it->second.get())->has(assetName);
}

}
