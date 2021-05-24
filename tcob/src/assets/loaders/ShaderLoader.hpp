// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/gfx/gl/GLShaderProgram.hpp>

namespace tcob::detail {
class ShaderLoader : public ResourceLoader<gl::ShaderProgram> {
public:
    explicit ShaderLoader(ResourceGroup& group);

    void register_wrapper(LuaScript& script) override;

protected:
    void do_unload(ResourcePtr<gl::ShaderProgram> res, bool greedy) override;
    auto do_reload(ResourcePtr<gl::ShaderProgram> res) -> bool override;

    void on_preparing() override;

private:
    struct ReloadInfo {
        std::string vertex;
        std::string fragment;
    };

    struct ShaderDef {
        ResourcePtr<gl::ShaderProgram> Res;
        std::unordered_set<std::string> defaultFor;
        ReloadInfo info;
    };

    std::function<ShaderDef*(const std::string&)> _funcNew;
    std::vector<std::unique_ptr<ShaderDef>> _cache;

    std::unordered_map<std::string, ReloadInfo> _reloadInfo;
};
}