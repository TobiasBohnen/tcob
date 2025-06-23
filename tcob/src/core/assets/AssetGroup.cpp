// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/assets/AssetGroup.hpp"

#include <utility>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/assets/AssetLoader.hpp"
#include "tcob/core/assets/Assets.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::assets {

group::group(string name)
    : _name {std::move(name)}
{
}

group::~group() = default;

auto group::name() const -> string const&
{
    return _name;
}

auto group::mount_point() const -> string
{
    return _name + "/";
}

auto group::asset_stats() const -> group_stats
{
    group_stats retValue {};
    for (auto const& [_, bucket] : _buckets) {
        bucket->asset_stats(retValue.Buckets[bucket->name()]);
    }
    return retValue;
}

void group::mount(path const& folderOrArchive) const
{
    io::mount(folderOrArchive, mount_point());
}

void group::load()
{
    auto files {io::enumerate(mount_point(), {.String = "*.assets.*"})};

    // script loading stage
    for (auto const& file : files) {
        auto const ext {io::get_extension(file)};

        if (!_loaderManagers.contains(ext)) {
            if (auto mgr {locate_service<loader_manager::factory>().create(ext, *this)}) {
                _loaderManagers[ext] = std::move(mgr);
            } else {
                continue;
            }
        }

        loader_manager*      alm {_loaderManagers[ext].get()};
        script_preload_event ev {.Path       = file,
                                 .Hasher     = io::file_hasher {file},
                                 .ShouldLoad = true};

        PreScriptLoad(ev);

        if (ev.ShouldLoad) { alm->load_script(file); }
    }

    // declaring stage
    for (auto& loaderMgr : _loaderManagers) { loaderMgr.second->declare(); }
    // preparing stage
    for (auto& loaderMgr : _loaderManagers) { loaderMgr.second->prepare(); }
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
    auto stats {asset_stats().Buckets};
    for (auto& [_, bucketStats] : stats) {
        if (bucketStats.Statuses[asset_status::Loading] > 0) { return false; }
    }

    return true;
}

auto group::loading_progress() const -> f32
{
    usize assetCount {0};
    f32   loaded {0.0f};

    auto stats {asset_stats().Buckets};
    for (auto& [_, bucketStats] : stats) {
        assetCount += bucketStats.Assets.size();
        loaded += static_cast<f32>(bucketStats.Statuses[asset_status::Loaded]);
    }

    if (assetCount == 0) { return 1.0f; }

    return loaded / static_cast<f32>(assetCount);
}

////////////////////////////////////////////////////////////

namespace detail {
    bucket_base::bucket_base(string assetName)
        : _assetName {std::move(assetName)}
    {
    }

    auto bucket_base::name() const -> string const&
    {
        return _assetName;
    }
}

}
