// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <tuple>
#include <vector>

#include "tcob/core/Serialization.hpp"
#include "tcob/core/easing/Tween.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API frame {
public:
    string       Name {};
    milliseconds Duration {};

    auto operator==(frame const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&frame::Name> {"name"},
            member<&frame::Duration> {"duration"}};
    }
};

////////////////////////////////////////////////////////////

class TCOB_API frame_animation final {
public:
    std::vector<frame> Frames {};

    auto operator()(f64 t) const -> string;

    auto get_frame_at(milliseconds time) const -> string;
    auto duration() const -> milliseconds;

    static inline char const* AssetName {"frame_animation"};

    auto operator==(frame_animation const& other) const -> bool = default;
};

using frame_animation_tween = callable_tween<frame_animation>;

}
