// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ParticleSystem.hpp"

#include <optional>
#include <tuple>

#include "tcob/core/Serialization.hpp"

namespace tcob::gfx {

auto constexpr particle_settings::Members()
{
    return std::tuple {
        member<&particle_settings::Speed> {"speed"},
        member<&particle_settings::Direction> {"direction"},

        member<&particle_settings::LinearAcceleration> {"linear_acceleration"},
        member<&particle_settings::LinearDamping> {"linear_dampling"},
        member<&particle_settings::RadialAcceleration> {"radial_acceleration"},
        member<&particle_settings::TangentialAcceleration> {"tangential_acceleration"},

        member<&particle_settings::Gravity> {"gravity"},

        member<&particle_settings::TextureRegion> {"texture_region"},
        member<&particle_settings::Colors> {"colors"},
        member<&particle_settings::Transparency> {"transparency"},

        member<&particle_settings::Lifetime> {"lifetime"},
    };
}
auto constexpr particle_emitter::emit_linear::Members() { return std::tuple {member<&particle_emitter::emit_linear::Rate> {"rate"}}; }

auto constexpr particle_emitter::emit_burst::Members()
{
    return std::tuple {
        member<&particle_emitter::emit_burst::Count> {"count"},
        member<&particle_emitter::emit_burst::Interval> {"interval"},
        member<&particle_emitter::emit_burst::Repeats> {"repeats"},
    };
}

auto constexpr particle_emitter::settings::Members()
{
    return std::tuple {
        member<&particle_emitter::settings::Template> {"template"},
        member<&particle_emitter::settings::Transient> {"transient"},
        member<&particle_emitter::settings::Pattern> {"pattern"},
        member<&particle_emitter::settings::SpawnArea> {"spawn_area"},
        member<&particle_emitter::settings::Lifetime, std::nullopt> {"lifetime"},
    };
}

}
