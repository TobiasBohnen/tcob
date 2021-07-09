// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <utility>

#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/gfx/drawables/ParticleSystem.hpp>

namespace tcob::detail {
class ParticleSystemLoader : public ResourceLoader<ParticleSystem> {
public:
    explicit ParticleSystemLoader(ResourceGroup& group);

    void register_wrapper(lua::Script& script) override;

protected:
    void on_preparing() override;
    void do_unload(ResourcePtr<ParticleSystem> res, bool greedy) override;

private:
    struct TemplateDef {
        SizeF size;
        std::pair<f32, f32> direction;
        std::pair<f32, f32> speed;
        std::pair<f32, f32> acceleration;
        std::pair<f32, f32> scale;
        std::pair<f32, f32> spin;
        std::pair<f32, f32> lifetime;
        std::pair<f32, f32> transparency;
    };

    struct EmitterDef {
        RectF spawnarea { RectF::Zero };
        f32 spawnrate { 1 };
        f64 lifetime { 1000 };
        bool loop { false };
        std::string texture;
        std::string partemplate;
    };

    struct SystemDef {
        ResourcePtr<ParticleSystem> Res;
        std::string material;
        std::vector<std::string> emitter;
    };

    template <typename Func>
    void define_template_function(lua::Wrapper<TemplateDef>& wrapper, const std::string& name, Func func)
    {
        wrapper.function(name, [func](TemplateDef* def, f32 min, std::optional<f32> max) {
            if (!max.has_value()) {
                max = min;
            }
            def->*func = { min, max.value() };
            return def;
        });
    }

    std::function<TemplateDef*(const std::string&)> _funcNewTemplate;
    std::function<EmitterDef*(const std::string&)> _funcNewEmitter;
    std::function<SystemDef*(const std::string&)> _funcNewSystem;

    std::unordered_map<std::string, std::unique_ptr<TemplateDef>> _templateCache;
    std::unordered_map<std::string, std::unique_ptr<EmitterDef>> _emitterCache;
    std::vector<std::unique_ptr<SystemDef>> _systemsCache;
};
}