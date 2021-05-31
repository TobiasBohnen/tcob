// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MaterialLoader.hpp"

#include <tcob/gfx/gl/GLShaderProgram.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>
#include <tcob/script/LuaScript.hpp>

namespace tcob::detail {

static const std::unordered_map<std::string, gl::BlendFunc> blendfunc {
    { "Zero", gl::BlendFunc::Zero },
    { "One", gl::BlendFunc::One },
    { "SrcColor", gl::BlendFunc::SrcColor },
    { "OneMinusScrColor", gl::BlendFunc::OneMinusScrColor },
    { "DstColor", gl::BlendFunc::DstColor },
    { "OneMinusDstColor", gl::BlendFunc::OneMinusDstColor },
    { "SrcAlpha", gl::BlendFunc::SrcAlpha },
    { "OneMinusSrcAlpha", gl::BlendFunc::OneMinusSrcAlpha },
    { "DstAlpha", gl::BlendFunc::DstAlpha },
    { "OneMinusDstAlpha", gl::BlendFunc::OneMinusDstAlpha },
    { "ConstantColor", gl::BlendFunc::ConstantColor },
    { "OneMinusConstantColor", gl::BlendFunc::OneMinusConstantColor },
    { "ConstantAlpha", gl::BlendFunc::ConstantAlpha },
    { "OneMinusConstantAlpha", gl::BlendFunc::OneMinusConstantAlpha },
};

static const std::unordered_map<std::string, gl::BlendEquation> blendequation {
    { "Add", gl::BlendEquation::Add },
    { "Subtract", gl::BlendEquation::Subtract },
    { "ReverseSubtract", gl::BlendEquation::ReverseSubtract },
    { "Min", gl::BlendEquation::Min },
    { "Max", gl::BlendEquation::Max },
};

MaterialLoader::MaterialLoader(ResourceGroup& group)
    : ResourceLoader<Material> { group }
{
}

void MaterialLoader::register_wrapper(LuaScript& script)
{
    // material
    _funcNew = [this](const std::string& s) {
        auto material { get_or_create_resource(s) };
        auto def { std::make_unique<MaterialDef>() };
        def->Res = material;

        auto retValue { def.get() };
        _cache.push_back(std::move(def));
        return retValue;
    };
    script.global_table()["material"] = _funcNew;

    auto& wrapper { script.create_wrapper<MaterialDef>("MaterialDef") };
    wrapper.function("texture", [](MaterialDef* def, const std::string& s) {
        def->texture = s;
        return def;
    });
    wrapper.function("shader", [](MaterialDef* def, const std::string& s) {
        def->shader = s;
        return def;
    });
    wrapper.function("blend_func", [](MaterialDef* def, const std::string& s, const std::string& d) {
        def->Res->SourceAlphaBlendFunc = blendfunc.at(s);
        def->Res->SourceColorBlendFunc = blendfunc.at(s);
        def->Res->DestinationAlphaBlendFunc = blendfunc.at(d);
        def->Res->DestinationColorBlendFunc = blendfunc.at(d);
        return def;
    });
    wrapper.function("separate_blend_func", [](MaterialDef* def, const std::string& cs, const std::string& cd, const std::string& as, const std::string& ad) {
        def->Res->SourceAlphaBlendFunc = blendfunc.at(as);
        def->Res->SourceColorBlendFunc = blendfunc.at(cs);
        def->Res->DestinationAlphaBlendFunc = blendfunc.at(ad);
        def->Res->DestinationColorBlendFunc = blendfunc.at(cd);
        return def;
    });
    wrapper.function("blend_equation", [](MaterialDef* def, const std::string& s) {
        def->Res->BlendEquation = blendequation.at(s);
        return def;
    });
}

void MaterialLoader::on_preparing()
{
    for (const auto& def : _cache) {
        def->Res->Shader = group().get<gl::ShaderProgram>(def->shader);
        def->Res->Texture = group().get<gl::Texture>(def->texture);

        set_resource_loaded(def->Res);
    }

    _cache.clear();
}

void MaterialLoader::do_unload(ResourcePtr<Material> res, bool greedy)
{
    if (greedy) {
        res->Shader.get()->unload(true);
        res->Texture.get()->unload(true);
    }
}
}