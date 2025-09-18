// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/assets/AssetLibrary.hpp"

#include <algorithm>
#include <memory>

#include "tcob/core/Logger.hpp"
#include "tcob/core/assets/AssetGroup.hpp"
#include "tcob/core/assets/Assets.hpp"

namespace tcob::assets {

library::~library()
{
    destroy_all_groups();
}

auto library::create_or_get_group(string const& groupName) -> group&
{
    if (!_groups.contains(groupName)) {
        _groups[groupName] = std::make_unique<group>(groupName);
    }

    return *_groups[groupName].get();
}

auto library::get_group(string const& groupName) const -> group*
{
    auto it {_groups.find(groupName)};
    return it != _groups.end() ? it->second.get() : nullptr;
}

auto library::has_group(string const& groupName) const -> bool
{
    return _groups.contains(groupName);
}

void library::load_group(string const& groupName)
{
    auto it {_groups.find(groupName)};
    if (it != _groups.end()) {
        it->second->load();
    }
}

void library::load_all_groups()
{
    for (auto& [_, group] : _groups) {
        group->load();
    }
}

void library::unload_group(string const& group)
{
    auto it {_groups.find(group)};
    if (it != _groups.end()) {
        it->second->unload();
    }
}

void library::unload_all_groups()
{
    for (auto& [_, group] : _groups) {
        group->unload();
    }
}

void library::destroy_group(string const& groupName)
{
    auto it {_groups.find(groupName)};
    if (it != _groups.end()) {
        it->second->destroy();
        _groups.erase(it);
    }
}

void library::destroy_all_groups()
{
    for (auto& [_, group] : _groups) {
        group->destroy();
    }

    _groups.clear();
}

auto library::loading_progress() const -> f32
{
    f32 assets {0.0f};
    f32 loaded {0.0f};

    for (auto const& [g, group] : _groups) {
        auto stats {group->asset_stats().Buckets};
        for (auto& [b, bucketStats] : stats) {
            assets += static_cast<f32>(bucketStats.Assets.size());
            loaded += static_cast<f32>(bucketStats.Statuses[asset_status::Loaded]);
        }
    }

    if (assets == 0.0f) {
        return 0.0f;
    }

    return loaded / assets;
}

auto library::is_loading_complete() const -> bool
{
    return std::ranges::all_of(_groups,
                               [](auto&& value) { return value.second->is_loading_complete(); });
}

auto library::asset_stats(string const& group) const -> group_stats
{
    auto it {_groups.find(group)};
    if (it == _groups.end()) {
        logger::Error("AssetLibrary: group '{}' not found.", group);
        return {};
    }

    return it->second->asset_stats();
}

}
