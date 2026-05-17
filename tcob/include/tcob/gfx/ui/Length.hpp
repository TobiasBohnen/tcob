// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <tuple>

#include "tcob/core/Rect.hpp"
#include "tcob/core/Serialization.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

class TCOB_API length final {
    friend auto operator/(length const& left, f32 right) -> length;

public:
    enum class type : u8 {
        Relative,
        Absolute
    };

    length() = default;
    length(f32 val, type t);

    f32  Value {0};
    type Type {type::Absolute};

    auto calc(f32 min, f32 refSize) const -> f32;
    auto calc(f32 refSize) const -> f32;

    auto operator==(length const& other) const -> bool = default;
    auto operator-() const -> length;

    static auto Lerp(length const& from, length const& to, f64 step) -> length;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&length::Value> {"value"},
            member<&length::Type> {"type"}};
    }
};

auto operator/(length const& left, f32 right) -> length;

////////////////////////////////////////////////////////////

class TCOB_API thickness final {
public:
    thickness() = default;
    thickness(length all);
    thickness(length lr, length tb);
    thickness(length l, length r, length t, length b);

    length Left {};
    length Right {};
    length Top {};
    length Bottom {};

    auto operator==(thickness const& other) const -> bool = default;

    static auto Lerp(thickness const& from, thickness const& to, f64 step) -> thickness;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&thickness::Left> {"left"},
            member<&thickness::Right> {"right"},
            member<&thickness::Top> {"top"},
            member<&thickness::Bottom> {"bottom"}};
    }
};

auto operator-(rect_f const& left, thickness const& right) -> rect_f;
auto operator-=(rect_f& left, thickness const& right) -> rect_f&;
auto operator+(rect_f const& left, thickness const& right) -> rect_f;
auto operator+=(rect_f& left, thickness const& right) -> rect_f&;

////////////////////////////////////////////////////////////

class TCOB_API dimensions final {
public:
    length Width {1, length::type::Relative};
    length Height {1, length::type::Relative};

    auto calc(size_f refSize) const -> size_f;

    auto operator==(dimensions const& other) const -> bool = default;

    static auto Lerp(dimensions const& from, dimensions const& to, f64 step) -> dimensions;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&dimensions::Width> {"width"},
            member<&dimensions::Height> {"height"}};
    }
};

}

namespace tcob::literals {
inline auto operator""_pct(long double val) -> ui::length
{
    return ui::length {static_cast<f32>(val / 100.0), ui::length::type::Relative};
}
inline auto operator""_pct(unsigned long long int val) -> ui::length
{
    return ui::length {static_cast<f32>(val) / 100.0f, ui::length::type::Relative};
}
inline auto operator""_px(unsigned long long int val) -> ui::length
{
    return ui::length {static_cast<f32>(val), ui::length::type::Absolute};
}
}

#include "Length.inl"
