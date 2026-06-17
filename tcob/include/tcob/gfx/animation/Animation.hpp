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

class TCOB_API sprite_animation final {
public:
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

    ////////////////////////////////////////////////////////////

    explicit sprite_animation(std::vector<frame> const& frames);

    auto get_frame_at(milliseconds time) const -> string_view;
    auto duration() const -> milliseconds;
    auto is_empty() const -> bool;

    auto operator()(f64 t) const -> string_view;

    static inline char const* AssetName {"sprite_animation"};

    auto operator==(sprite_animation const& other) const -> bool = default;

private:
    std::vector<frame> _frames;
    milliseconds       _duration {};
};

using sprite_animation_tween = callable_tween<sprite_animation>;

////////////////////////////////////////////////////////////

class TCOB_API skeletal_animation final {
public:
    ////////////////////////////////////////////////////////////

    class TCOB_API frame final {
    public:
        string       Name {};
        milliseconds Duration {};

        point_f  Translation {};
        radian_f Rotation {};
        size_f   Scale {1.0f, 1.0f};

        auto operator==(frame const& other) const -> bool = default;

        static auto constexpr Members()
        {
            return std::tuple {
                member<&frame::Name> {"name"},
                member<&frame::Duration> {"duration"},
                member<&frame::Translation> {"translation"},
                member<&frame::Rotation> {"rotation"},
                member<&frame::Scale> {"scale"}};
        }
    };

    class TCOB_API bone final {
    public:
        string Name {};
        string Parent {};

        transform          Rest {};
        std::vector<frame> Track {};

        auto operator==(bone const& other) const -> bool = default;
        static auto constexpr Members()
        {
            return std::tuple {
                member<&bone::Name> {"name"},
                member<&bone::Parent> {"parent"},
                member<&bone::Rest> {"rest"},
                member<&bone::Track> {"track"}};
        }
    };

    class TCOB_API pose final {
    public:
        transform   Transform {};
        string_view Region {};

        auto operator==(pose const& other) const -> bool = default;
    };
    using poses = std::vector<pose>;

    ////////////////////////////////////////////////////////////

    explicit skeletal_animation(std::vector<bone> const& bones);

    auto duration() const -> milliseconds;
    auto bone_count() const -> isize;
    auto is_empty() const -> bool;

    auto operator()(f64 t) const -> poses;

    auto operator==(skeletal_animation const& other) const -> bool = default;

    // static inline char const* AssetName {"skeletal_animation"};

private:
    void compute_pose(std::map<isize, transform> const& locals, poses& poses) const;

    std::vector<bone>  _bones {};
    milliseconds       _duration {};
    std::vector<isize> _parentIndices {};
};

using skeletal_animation_tween = callable_tween<skeletal_animation>;

}
