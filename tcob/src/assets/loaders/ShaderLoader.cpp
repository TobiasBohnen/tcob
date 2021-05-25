// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ShaderLoader.hpp"

#include <tcob/gfx/Font.hpp>
#include <tcob/gfx/gl/GLWindow.hpp>
#include <tcob/script/LuaScript.hpp>

namespace tcob::detail {
ShaderLoader::ShaderLoader(ResourceGroup& group)
    : ResourceLoader<gl::ShaderProgram> { group }
{
}

void ShaderLoader::register_wrapper(LuaScript& script)
{
    // shader
    _funcNew = [this](const std::string& s) {
        auto shader { get_or_create_resource(s) };
        auto def { std::make_unique<ShaderDef>() };
        def->Res = shader;

        auto retValue { def.get() };
        _cache.push_back(std::move(def));
        return retValue;
    };
    script.global_table()["shader"] = _funcNew;

    auto& wrapper { script.create_wrapper<ShaderDef>("ShaderDef") };
    wrapper.function("vertex", [](ShaderDef* def, const std::string& s) {
        def->info.vertex = s;
        return def;
    });
    wrapper.function("fragment", [](ShaderDef* def, const std::string& s) {
        def->info.fragment = s;
        return def;
    });
    wrapper.function("default_for", [](ShaderDef* def, const std::unordered_set<std::string>& s) {
        def->defaultFor = s;
        return def;
    });
}

void ShaderLoader::do_unload(ResourcePtr<gl::ShaderProgram> res, bool)
{
    _reloadInfo.erase(res.get()->name());
}

auto ShaderLoader::do_reload(ResourcePtr<gl::ShaderProgram> res) -> bool
{
    if (!_reloadInfo.contains(res.get()->name())) {
        return false;
    }

    auto& info { _reloadInfo[res.get()->name()] };
    const std::string vertSource { FileSystem::read_as_string(group().mount_point() + info.vertex) };
    const std::string fragSource { FileSystem::read_as_string(group().mount_point() + info.fragment) };
    return res->create(vertSource.c_str(), fragSource.c_str());
}

void ShaderLoader::on_preparing()
{
    if (!_cache.empty()) {
        for (auto& def : _cache) {
            const std::string vertSource { FileSystem::read_as_string(group().mount_point() + def->info.vertex) };
            const std::string fragSource { FileSystem::read_as_string(group().mount_point() + def->info.fragment) };

            def->Res->create(vertSource.c_str(), fragSource.c_str());

            if (def->defaultFor.contains("Font")) {
                Font::DefaultShader = def->Res;
            }
            if (def->defaultFor.contains("UI")) {
                // ui::UILayer::DefaultShader = shader->Obj;
            }
            if (def->defaultFor.contains("Window")) {
                gl::Window::DefaultShader = def->Res;
            }

            _reloadInfo[def->Res.get()->name()] = def->info;

            set_resource_loaded(def->Res);
        }

        _cache.clear();
    }
}
}