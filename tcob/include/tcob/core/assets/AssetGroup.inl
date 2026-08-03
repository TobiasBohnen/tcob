// Copyright (c) 2026 Tobias Bohnen
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
    auto [it, inserted] {_objects.emplace(name, typename decltype(_objects)::mapped_type {})};
    auto& assetEntry {it->second};
    auto  obj {std::make_shared<R>(std::forward<Args>(args)...)};
    if (inserted) {
        assetEntry = {obj, asset_ptr<T> {std::make_shared<asset<T>>(name, obj)}};
    } else {
        assetEntry.first = obj;
        assetEntry.second.get()->reset(obj);
    }
    return assetEntry.second;
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
    obj.second.get()->Status = asset_status::Unloaded;
}

template <typename T>
inline void bucket<T>::asset_stats(bucket_stats& out) const
{
    for (auto& [name, ptr] : _objects) {
        auto const st {ptr.second.get()->Status};
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
inline auto group::bucket() -> assets::bucket<T>*
{
    auto [it, inserted] {_buckets.emplace(typeid(T), nullptr)};
    if (inserted) {
        it->second = std::make_unique<assets::bucket<T>>(*this);
    }
    return dynamic_cast<assets::bucket<T>*>(it->second.get());
}

template <typename T>
inline auto group::bucket() const -> assets::bucket<T> const*
{
    return dynamic_cast<assets::bucket<T> const*>(_buckets.at(typeid(T)).get());
}

template <typename T, typename... Args>
inline auto group::create(string const& assetName, Args&&... args) -> asset_ptr<T>
{
    return bucket<T>()->create(assetName, std::forward<Args>(args)...);
}

template <typename T>
inline auto group::get(string const& assetName) const -> asset_ptr<T>
{
    if (!has<T>(assetName)) {
        logger::Error("asset_group '{}': asset '{}' not found", _name, assetName);
        return asset_ptr<T> {nullptr};
    }

    return bucket<T>()->get(assetName);
}

template <typename T>
inline auto group::has(string const& assetName) const -> bool
{
    auto it {_buckets.find(typeid(T))};
    if (it == _buckets.end()) { return false; }

    return bucket<T>()->has(assetName);
}

}
