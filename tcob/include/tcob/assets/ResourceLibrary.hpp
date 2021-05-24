// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <functional>
#include <typeindex>
#include <unordered_map>

#include <tcob/assets/Resource.hpp>
#include <tcob/script/LuaScript.hpp>
#include <tcob/thirdparty/sigslot/signal.hpp>

namespace tcob {

class ResourceGroup final {
public:
    explicit ResourceGroup(std::string name);
    ~ResourceGroup() = default;

    sigslot::signal<> Loading;
    sigslot::signal<> Preparing;
    sigslot::signal<> Unloading;
    sigslot::signal<> Updating;

    template <typename T>
    auto get(const std::string& resname) const -> ResourcePtr<T>;
    template <typename T>
    auto has(const std::string& resname) const -> bool;

    auto mount_point() const -> std::string;

    template <typename T>
    void register_loader(std::unique_ptr<ResourceLoader<T>> loader);

    void scan_for_scripts();

    void load();
    void unload();

    void update();
    auto resource_state() const -> std::unordered_map<ResourceState, u32>;

private:
    std::string _name;
    LuaScript _luaScript;
    std::unordered_map<std::type_index, std::unique_ptr<ResourceLoaderBase>> _loaders;

    std::vector<std::string> _groupScriptFiles;
};

template <typename T>
auto ResourceGroup::get(const std::string& resname) const -> ResourcePtr<T>
{
    if (!has<T>(resname)) {
        return ResourcePtr<T> { nullptr };
    }

    return static_cast<ResourceLoader<T>*>(_loaders.at(typeid(T)).get())->get(resname);
}

template <typename T>
auto ResourceGroup::has(const std::string& resname) const -> bool
{
    if (!_loaders.contains(typeid(T))) {
        return false;
    }

    return static_cast<ResourceLoader<T>*>(_loaders.at(typeid(T)).get())->has(resname);
}

template <typename T>
void ResourceGroup::register_loader(std::unique_ptr<ResourceLoader<T>> loader)
{
    loader->register_wrapper(_luaScript);
    _loaders[typeid(T)] = std::move(loader);
}

////////////////////////////////////////////////////////////

class ResourceLibrary final {
public:
    ResourceLibrary();
    ~ResourceLibrary();

    template <typename T>
    auto get(const std::string& group, const std::string& resname) const -> ResourcePtr<T>;
    template <typename T>
    auto has(const std::string& group, const std::string& resname) const -> bool;

    void mount(const std::string& group, const std::string& path);

    void load_all_groups();
    void load_group(const std::string& group);
    void unload_all_groups();
    void unload_group(const std::string& group);

    void update();

    auto resource_state(const std::string& group) const -> std::unordered_map<ResourceState, u32>;

private:
    std::unordered_map<std::string, std::unique_ptr<ResourceGroup>> _groups;
};

template <typename T>
auto ResourceLibrary::get(const std::string& group, const std::string& resname) const -> ResourcePtr<T>
{
    if (!has<T>(group, resname)) {
        return ResourcePtr<T> { nullptr };
    }

    return _groups.at(group)->get<T>(resname);
}

template <typename T>
auto ResourceLibrary::has(const std::string& group, const std::string& resname) const -> bool
{
    if (!_groups.contains(group)) {
        return false;
    }

    return _groups.at(group)->has<T>(resname);
}

}