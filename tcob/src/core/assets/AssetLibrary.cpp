// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/assets/AssetLibrary.hpp"

#include <algorithm>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::assets {

library::library() = default;

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

auto library::get_loading_progress() const -> f32
{
    f32 assets {0.0f};
    f32 loaded {0.0f};

    for (auto const& [g, group] : _groups) {
        auto stats {group->get_asset_stats().Buckets};
        for (auto& [b, bucketStats] : stats) {
            assets += bucketStats.Assets.size();
            loaded += static_cast<f32>(bucketStats.Statuses[status::Loaded]);
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

auto library::get_asset_stats(string const& group) const -> group_stats
{
    auto it {_groups.find(group)};
    if (it == _groups.end()) {
        logger::Error("AssetLibrary: group '{}' not found.", group);
        return {};
    }

    return it->second->get_asset_stats();
}

////////////////////////////////////////////////////////////

group::group(string name)
    : _name {std::move(name)}
{
}

auto group::get_name() const -> string const&
{
    return _name;
}

auto group::get_mount_point() const -> string
{
    return _name + "/";
}

auto group::get_asset_stats() const -> group_stats
{
    group_stats retValue {};
    for (auto const& [_, bucket] : _buckets) {
        bucket->get_asset_stats(retValue.Buckets[bucket->get_name()]);
    }
    return retValue;
}

void group::mount(path const& folderOrArchive) const
{
    io::mount(folderOrArchive, get_mount_point());
}

void group::load()
{
    auto files {io::enumerate(get_mount_point(), {"*.assets.*"})};

    // load script files
    for (auto const& file : files) {
        auto const ext {io::get_extension(file)};

        if (!_loaderManagers.contains(ext)) {
            if (auto mgr {locate_service<loader_manager::factory>().create(ext, *this)}) {
                _loaderManagers[ext] = std::move(mgr);
            } else {
                continue;
            }
        }

        loader_manager* alm {_loaderManagers[ext].get()};
        if (alm) {
            script_preload_event ev {.Path       = file,
                                     .Hasher     = io::file_hasher {file},
                                     .ShouldLoad = true};

            PreScriptLoad(ev);

            if (ev.ShouldLoad) {
                alm->load(file);
            }
        } else {
            // TODO: log error
        }
    }

    for (auto& loaderMgr : _loaderManagers) {
        loaderMgr.second->declare();
    }

    for (auto& loaderMgr : _loaderManagers) {
        loaderMgr.second->prepare();
    }
}

void group::unload()
{
    for (auto& [_, bucket] : _buckets) {
        bucket->unload_all();
    }
}

void group::destroy()
{
    for (auto& [_, bucket] : _buckets) {
        bucket->destroy_all();
    }
}

auto group::is_loading_complete() const -> bool
{
    auto stats {get_asset_stats().Buckets};
    for (auto& [_, bucketStats] : stats) {
        if (bucketStats.Statuses[status::Loading] > 0) { return false; }
    }

    return true;
}

auto group::get_loading_progress() const -> f32
{
    f32 assets {0.0f};
    f32 loaded {0.0f};

    auto stats {get_asset_stats().Buckets};
    for (auto& [_, bucketStats] : stats) {
        assets += bucketStats.Assets.size();
        loaded += static_cast<f32>(bucketStats.Statuses[status::Loaded]);
    }

    if (assets == 0.0f) {
        return 0.0f;
    }

    return loaded / assets;
}

////////////////////////////////////////////////////////////

namespace detail {
    bucket_base::bucket_base(string assetName)
        : _assetName {std::move(assetName)}
    {
    }

    auto bucket_base::get_name() const -> string const&
    {
        return _assetName;
    }
}

////////////////////////////////////////////////////////////

void loader_manager::declare()
{
    for (auto& loader : _loaders) {
        loader->declare();
    }
}

void loader_manager::prepare()
{
    for (auto& loader : _loaders) {
        loader->prepare();
    }
}

void loader_manager::add_loader(std::unique_ptr<detail::loader_base> loader)
{
    _loaders.push_back(std::move(loader));
}

}
