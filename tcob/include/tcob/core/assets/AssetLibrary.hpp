// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>
#include <vector>

#include "tcob/core/FlatMap.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/TypeFactory.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::assets {

////////////////////////////////////////////////////////////

struct stat {
    status Status;
    i32    UseCount;
};

struct bucket_stats {
    flat_map<string, stat> Assets;
    flat_map<status, i32>  Statuses;
};

struct group_stats {
    flat_map<string, bucket_stats> Buckets;
};

////////////////////////////////////////////////////////////

namespace detail {
    class bucket_base {
    public:
        explicit bucket_base(string assetName);
        virtual ~bucket_base() = default;

        auto get_name() const -> string const&;

        void virtual get_asset_stats(bucket_stats& out) const = 0;

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
    explicit bucket(group& groupName);

    template <typename R = T, typename... Args>
    auto create_or_get(string const& name, loader<T>* loader, Args&&... args) -> assets::asset_ptr<T>;

    auto get(string const& name) const -> assets::asset_ptr<T>;

    auto has(string const& name) const -> bool;

    void get_asset_stats(bucket_stats& out) const override;

    void destroy_all() override;
    void destroy(string const& name) override;

    void unload_all() override;
    void unload(string const& name) override;

protected:
    auto get_group() -> group&;

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

    signal<script_preload_event> PreScriptLoad;

    auto get_name() const -> string const&;
    auto get_mount_point() const -> string;
    auto get_asset_stats() const -> group_stats;

    auto get_loading_progress() const -> f32;
    auto is_loading_complete() const -> bool;

    template <typename T>
    void add_bucket();

    template <typename T>
    auto get_bucket() -> bucket<T>*;

    template <typename T>
    auto get(string const& assetName) const -> assets::asset_ptr<T>;

    template <typename T>
    auto has(string const& assetName) const -> bool;

    void mount(path const& folderOrArchive) const;

    void load();
    void unload();
    void destroy();

private:
    string                                                 _name;
    flat_map<string, std::unique_ptr<detail::bucket_base>> _buckets;
    flat_map<string, std::unique_ptr<loader_manager>>      _loaderManagers;
};

////////////////////////////////////////////////////////////

class TCOB_API library final : public non_copyable {
public:
    library();
    ~library();

    auto get_loading_progress() const -> f32;
    auto is_loading_complete() const -> bool;

    auto create_or_get_group(string const& groupName) -> group&;
    auto get_group(string const& groupName) const -> group*;
    auto has_group(string const& groupName) const -> bool;

    void load_group(string const& groupName);
    void load_all_groups();

    void unload_group(string const& groupName);
    void unload_all_groups();

    void destroy_group(string const& groupName);
    void destroy_all_groups();

    auto get_asset_stats(string const& groupName) const -> group_stats;

    static inline char const* service_name {"asset_library"}; // TODO: remove

private:
    flat_map<string, std::unique_ptr<group>> _groups {};
};

////////////////////////////////////////////////////////////

namespace detail {
    class loader_base {
    public:
        virtual ~loader_base() = default;

        void virtual declare() { }
        void virtual prepare() { }
    };
}

////////////////////////////////////////////////////////////

template <typename T>
class loader : public detail::loader_base {
public:
    loader(group& group);

    void unload(asset<T>& asset, bool greedy);

    auto reload(asset<T>& asset) -> bool;

protected:
    void virtual unload_asset(asset<T>& asset, bool greedy) = 0;
    auto virtual reload_asset(asset<T>& asset) -> bool;

    auto get_group() -> group&;

    auto get_bucket() -> bucket<T>*;

    void set_asset_status(asset_ptr<T> asset, status status);

private:
    group& _group;
};
////////////////////////////////////////////////////////////

class TCOB_API loader_manager : public non_copyable {
public:
    struct factory : public type_factory<std::unique_ptr<loader_manager>, group&> {
        static inline char const* service_name {"assets::loader_manager::factory"};
    };

    loader_manager()          = default;
    virtual ~loader_manager() = default;

    void virtual load(path const& file) = 0;
    void declare();
    void prepare();

protected:
    void add_loader(std::unique_ptr<detail::loader_base> loader);

private:
    std::vector<std::unique_ptr<detail::loader_base>> _loaders;
};

}

#include "AssetLibrary.inl"
