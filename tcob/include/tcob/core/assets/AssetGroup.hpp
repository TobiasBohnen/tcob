// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <unordered_map>
#include <utility>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/assets/Assets.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::assets {
////////////////////////////////////////////////////////////

namespace detail {
    class TCOB_API bucket_base {
    public:
        explicit bucket_base(string assetName);
        virtual ~bucket_base() = default;

        auto name() const -> string const&;

        void virtual asset_stats(bucket_stats& out) const = 0;

        void virtual destroy_all()               = 0;
        void virtual destroy(string const& name) = 0;

        void virtual unload_all()               = 0;
        void virtual unload(string const& name) = 0;

    private:
        string _assetName;
    };
}

////////////////////////////////////////////////////////////

template <typename T>
class bucket final : public detail::bucket_base {
public:
    explicit bucket(group& parent);

    template <typename R = T, typename... Args>
    auto create(string const& name, Args&&... args) -> assets::asset_ptr<T>;

    auto get(string const& name) const -> assets::asset_ptr<T>;

    auto has(string const& name) const -> bool;

    void asset_stats(bucket_stats& out) const override;

    void destroy_all() override;
    void destroy(string const& name) override;

    void unload_all() override;
    void unload(string const& name) override;

protected:
    auto parent() -> group&;

private:
    group& _group;

    std::unordered_map<string, std::pair<std::shared_ptr<T>, assets::asset_ptr<T>>> _objects;
};

////////////////////////////////////////////////////////////

struct script_preload_event {
    string          Path;
    io::file_hasher Hasher;
    bool            ShouldLoad;
};

////////////////////////////////////////////////////////////

class TCOB_API group final : public non_copyable {
public:
    explicit group(string name);
    ~group();

    signal<script_preload_event> PreScriptLoad;

    auto name() const -> string const&;
    auto mount_point() const -> string;
    auto asset_stats() const -> group_stats;

    auto loading_progress() const -> f32;
    auto is_loading_complete() const -> bool;

    template <typename T>
    void add_bucket();

    template <typename T>
    auto bucket() -> assets::bucket<T>*;

    template <typename T>
    auto get(string const& assetName) const -> assets::asset_ptr<T>;

    template <typename T>
    auto has(string const& assetName) const -> bool;

    void mount(path const& folderOrArchive) const;

    void load();
    void unload();
    void destroy();

private:
    string                                                           _name;
    std::unordered_map<string, std::unique_ptr<detail::bucket_base>> _buckets;
    std::unordered_map<string, std::unique_ptr<loader_manager>>      _loaderManagers;
};

}

#include "AssetGroup.inl"
