// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ParticleSystemLoader.hpp"

#include <tcob/gfx/Material.hpp>
#include <tcob/script/LuaScript.hpp>

namespace tcob::detail {
ParticleSystemLoader::ParticleSystemLoader(ResourceGroup& group)
    : ResourceLoader<ParticleSystem> { group }
{
}

void ParticleSystemLoader::register_wrapper(lua::Script& script)
{
    // template
    _funcNewTemplate = [this](const std::string& s) {
        auto def { std::make_unique<TemplateDef>() };

        auto retValue { def.get() };
        _templateCache[s] = std::move(def);
        return retValue;
    };
    script.global_table()["particle_template"] = _funcNewTemplate;

    auto& templatewrap { script.create_wrapper<TemplateDef>("TemplateDef") };
    define_template_function(templatewrap, "direction", &TemplateDef::direction);
    define_template_function(templatewrap, "speed", &TemplateDef::speed);
    define_template_function(templatewrap, "acceleration", &TemplateDef::acceleration);
    define_template_function(templatewrap, "scale", &TemplateDef::scale);
    define_template_function(templatewrap, "spin", &TemplateDef::spin);
    define_template_function(templatewrap, "lifetime", &TemplateDef::lifetime);
    define_template_function(templatewrap, "transparency", &TemplateDef::transparency);
    templatewrap.function("size", [](TemplateDef* def, SizeF size) {
        def->size = size;
        return def;
    });

    // emitter
    _funcNewEmitter = [this](const std::string& s) {
        auto def { std::make_unique<EmitterDef>() };

        auto retValue { def.get() };
        _emitterCache[s] = std::move(def);
        return retValue;
    };
    script.global_table()["particle_emitter"] = _funcNewEmitter;

    auto& emitterwrap { script.create_wrapper<EmitterDef>("EmitterDef") };
    emitterwrap.function("spawnarea", [](EmitterDef* def, RectF val) {
        def->spawnarea = val;
        return def;
    });
    emitterwrap.function("lifetime", [](EmitterDef* def, f64 val) {
        def->lifetime = val;
        return def;
    });
    emitterwrap.function("loop", [](EmitterDef* def, bool val) {
        def->loop = val;
        return def;
    });
    emitterwrap.function("spawnrate", [](EmitterDef* def, f32 val) {
        def->spawnrate = val;
        return def;
    });
    emitterwrap.function("texture", [](EmitterDef* def, const std::string& val) {
        def->texture = val;
        return def;
    });
    emitterwrap.function("template", [](EmitterDef* def, const std::string& val) {
        def->partemplate = val;
        return def;
    });

    // particle system
    _funcNewSystem = [this](const std::string& s) {
        auto system { get_or_create_resource(s) };
        auto def { std::make_unique<SystemDef>() };
        def->Res = system;

        auto retValue { def.get() };
        _systemsCache.push_back(std::move(def));
        return retValue;
    };
    script.global_table()["particle_system"] = _funcNewSystem;

    auto& systemwrap { script.create_wrapper<SystemDef>("SystemDef") };
    systemwrap.function("material", [](SystemDef* def, const std::string& val) {
        def->material = val;
        return def;
    });
    systemwrap.function("emitters", [](SystemDef* def, std::vector<std::string>& val) {
        def->emitter = val;
        return def;
    });
}

void ParticleSystemLoader::on_preparing()
{
    for (auto& def : _systemsCache) {
        def->Res->material(group().get<Material>(def->material));
        def->Res->remove_all_emitters();

        for (auto& emitterName : def->emitter) {
            auto& emitter { def->Res->create_emitter() };
            auto* emitterDef { _emitterCache[emitterName].get() };
            emitter.spawnarea(emitterDef->spawnarea);
            emitter.lifetime(MilliSeconds { emitterDef->lifetime });
            emitter.loop(emitterDef->loop);
            emitter.spawnrate(emitterDef->spawnrate);
            emitter.texture_region(emitterDef->texture);

            auto* templateDef { _templateCache[emitterDef->partemplate].get() };
            emitter.particle_direction(templateDef->direction.first, templateDef->direction.second);
            emitter.particle_speed(templateDef->speed.first, templateDef->speed.second);
            emitter.particle_acceleration(templateDef->acceleration.first, templateDef->acceleration.second);
            emitter.particle_scale(templateDef->scale.first, templateDef->scale.second);
            emitter.particle_spin(templateDef->spin.first, templateDef->spin.second);
            emitter.particle_lifetime(MilliSeconds { templateDef->lifetime.first }, MilliSeconds { templateDef->lifetime.second });
            emitter.particle_transparency(templateDef->transparency.first, templateDef->transparency.second);
            emitter.particle_size(templateDef->size);
        }

        set_resource_loaded(def->Res);
    }

    _systemsCache.clear();
    _emitterCache.clear();
    _templateCache.clear();
}

void ParticleSystemLoader::do_unload(ResourcePtr<ParticleSystem> res, bool greedy)
{
    if (greedy)
        res->material().get()->unload(true);
}
}