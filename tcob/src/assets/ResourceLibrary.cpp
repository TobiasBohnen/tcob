// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/assets/ResourceLibrary.hpp>

#include "loaders/AnimationLoader.hpp"
#include "loaders/AudioLoader.hpp"
#include "loaders/CursorLoader.hpp"
#include "loaders/FontLoader.hpp"
#include "loaders/MaterialLoader.hpp"
#include "loaders/ParticleSystemLoader.hpp"
#include "loaders/ShaderLoader.hpp"
#include "loaders/TextureLoader.hpp"
#include "loaders/WebpAnimationLoader.hpp"

namespace tcob {
const std::string& BOOTSTRAP_FILE { "bootstrap.lua" };

ResourceLibrary::ResourceLibrary()
{
    LuaScript script;
    if (FileSystem::exists(BOOTSTRAP_FILE)) {
        auto result { script.run_file<std::unordered_map<std::string, std::vector<std::string>>>(BOOTSTRAP_FILE) };
        if (result.State == LuaResultState::Ok) {
            for (auto& [group, folders] : result.Value) {
                for (auto& folder : folders) {
                    mount(group, folder);
                }
            }
        }
    }
}

ResourceLibrary::~ResourceLibrary()
{
    unload_all_groups();
}

void ResourceLibrary::mount(const std::string& group, const std::string& path)
{
    FileSystem::mount(path, group + "/");
    if (!_groups.contains(group))
        _groups[group] = std::make_unique<ResourceGroup>(group);
}

void ResourceLibrary::load_all_groups()
{
    for (auto& [k, v] : _groups) {
        load_group(k);
    }
}

void ResourceLibrary::load_group(const std::string& group)
{
    if (!_groups.contains(group)) {
        return;
    }

    _groups[group]->load();
}

void ResourceLibrary::unload_all_groups()
{
    while (!_groups.empty()) {
        unload_group((*_groups.begin()).first);
    }
}

void ResourceLibrary::unload_group(const std::string& group)
{
    if (!_groups.contains(group)) {
        return;
    }

    _groups[group]->unload();
    _groups.erase(group);
}

void ResourceLibrary::update()
{
    for (auto& [_, group] : _groups) {
        group->update();
    }
}

auto ResourceLibrary::resource_state(const std::string& group) const -> std::unordered_map<ResourceState, u32>
{
    if (!_groups.contains(group)) {
        return {};
    }

    return _groups.at(group)->resource_state();
}

////////////////////////////////////////////////////////////
ResourceGroup::ResourceGroup(std::string name)
    : _name { std::move(name) }
{
    _luaScript.open_libraries();
    register_loader<gl::ShaderProgram>(std::make_unique<detail::ShaderLoader>(*this));
    register_loader<gl::Texture>(std::make_unique<detail::TextureLoader>(*this));
    register_loader<Material>(std::make_unique<detail::MaterialLoader>(*this));
    register_loader<Cursor>(std::make_unique<detail::CursorLoader>(*this));
    register_loader<ParticleSystem>(std::make_unique<detail::ParticleSystemLoader>(*this));
    register_loader<Font>(std::make_unique<detail::FontLoader>(*this));
    register_loader<FrameAnimation>(std::make_unique<detail::AnimationLoader>(*this));
    register_loader<WebpAnimation>(std::make_unique<detail::WebpAnimationLoader>(*this));
    register_loader<Music>(std::make_unique<detail::MusicLoader>(*this));
    register_loader<Sound>(std::make_unique<detail::SoundLoader>(*this));
}

auto ResourceGroup::mount_point() const -> std::string
{
    return _name + "/";
}

void ResourceGroup::scan_for_scripts()
{
    _groupScriptFiles = FileSystem::enumerate(_name + "/", "*.lua");
}

void ResourceGroup::load()
{
    scan_for_scripts();

    Loading();
    for (const auto& file : _groupScriptFiles) {
        auto result { _luaScript.run_file(file).State };
        if (result != LuaResultState::Ok) {
            //TODO: check error
        }
    }

    Preparing();
}

void ResourceGroup::unload()
{
    Unloading();
}

void ResourceGroup::update()
{
    Updating();
}

auto ResourceGroup::resource_state() const -> std::unordered_map<ResourceState, u32>
{
    std::unordered_map<ResourceState, u32> retValue {};
    for (auto& [_, loader] : _loaders) {
        loader->resource_state(retValue);
    }
    return retValue;
}

}