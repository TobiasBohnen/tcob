// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/assets/AssetGroup.hpp"

#include <utility>

#include "tcob/core/Logger.hpp"
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

        auto it {_loaderManagers.find(ext)};
        if (it == _loaderManagers.end()) {
            if (auto mgr {create_from_factory<loader_manager>(ext, *this)}) {
                it = _loaderManagers.emplace(ext, std::move(mgr)).first;
            } else {
                continue;
            }
        }
        loader_manager*      alm {it->second.get()};
        script_preload_event ev {.Path       = file,
                                 .Hasher     = io::file_hasher {file},
                                 .ShouldLoad = true};

        PreScriptLoad(ev);

        if (ev.ShouldLoad) { alm->load(file); }
    }

    // preparing stage
    for (auto& loaderMgr : _loaderManagers) { loaderMgr.second->prepare(); }
    // loading stage
    for (auto& loaderMgr : _loaderManagers) { loaderMgr.second->load(); }
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
    return loading_progress() == 1.0f;
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
        logger::Info("asset_bucket: '{}' created", _assetName);
    }

    auto bucket_base::name() const -> string const&
    {
        return _assetName;
    }
}

}
