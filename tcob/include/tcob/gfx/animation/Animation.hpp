// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <map>
#include <tuple>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Serialization.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/tweening/Tween.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API frame final {
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

class TCOB_API keyframe final {
public:
    milliseconds Timestamp {};

    point_f  Translation {};
    radian_f Rotation {};
    size_f   Scale {1.0f, 1.0f};

    auto operator==(keyframe const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&keyframe::Timestamp> {"timestamp"},
            member<&keyframe::Translation> {"translation"},
            member<&keyframe::Rotation> {"rotation"},
            member<&keyframe::Scale> {"scale"}};
    }
};

////////////////////////////////////////////////////////////

class TCOB_API frame_animation final {
public:
    explicit frame_animation(std::vector<frame> const& frames);

    auto get_frame_at(milliseconds time) const -> string_view;
    auto duration() const -> milliseconds;
    auto is_empty() const -> bool;

    auto operator()(f64 t) const -> string_view;

    static inline char const* AssetName {"frame_animation"};

    auto operator==(frame_animation const& other) const -> bool = default;

private:
    std::vector<frame> _frames;
    milliseconds       _duration {};
};

using frame_animation_tween = callable_tween<frame_animation>;

////////////////////////////////////////////////////////////

class TCOB_API skeletal_animation final {
public:
    using tracks = std::map<i32, std::vector<keyframe>>;

    struct bone {
        string    Name {};
        i32       Parent {-1};
        transform Rest {};

        auto operator==(bone const& other) const -> bool = default;

        static auto constexpr Members()
        {
            return std::tuple {
                member<&bone::Name> {"name"},
                member<&bone::Parent> {"parent"},
                member<&bone::Rest> {"rest"}};
        }
    };
    ////////////////////////////////////////////////////////////

    skeletal_animation(std::vector<bone> const& bones, tracks const& tracks);

    auto duration() const -> milliseconds;
    auto bone_count() const -> isize;
    auto is_empty() const -> bool;

    auto operator()(f64 t) const -> std::vector<transform>;

    auto operator==(skeletal_animation const& other) const -> bool = default;

    // static inline char const* AssetName {"skeletal_animation"};

private:
    void compute_pose(std::map<i32, transform> const& locals, std::vector<transform>& pose) const;

    std::vector<bone> _bones {};
    milliseconds      _duration {};
    tracks            _tracks {};
};

using skeletal_animation_tween = callable_tween<skeletal_animation>;

}
