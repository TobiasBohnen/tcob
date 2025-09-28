// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/StyleCollection.hpp"

#include <array>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

constexpr i32 FAIL_SCORE {std::numeric_limits<i32>::min()};

static auto variant_compare(auto const& lhs, auto const& rhs) -> op
{
    return std::visit([]<typename A, typename B>(A const& a, B const& b) -> op {
        if constexpr (!std::is_same_v<A, B>) {
            return op::NotEqual;
        } else if constexpr (std::is_enum_v<A>) {
            return (a == b) ? op::Equal : op::NotEqual;
        } else if constexpr (LessComparable<A>) {
            if (a == b) { return op::Equal; }
            return a < b ? op::Less : op::Greater;
        } else {
            return (a == b) ? op::Equal : op::NotEqual;
        }
    },
                      lhs, rhs);
}
using comparator_fn = bool (*)(widget_attribute_types const&, widget_attribute_types const&);

constexpr std::array<comparator_fn, 6> op_table {
    /* op::Equal */
    [](auto const& lhs, auto const& rhs) -> bool { return variant_compare(lhs, rhs) == op::Equal; },
    /* op::NotEqual */
    [](auto const& lhs, auto const& rhs) -> bool { return variant_compare(lhs, rhs) != op::Equal; },
    /* op::Greater */
    [](auto const& lhs, auto const& rhs) -> bool { return variant_compare(lhs, rhs) == op::Greater; },
    /* op::GreaterEqual */
    [](auto const& lhs, auto const& rhs) -> bool {
        auto const cmp {variant_compare(lhs, rhs)};
        return cmp == op::Greater || cmp == op::Equal;
    },
    /* op::Less */
    [](auto const& lhs, auto const& rhs) -> bool { return variant_compare(lhs, rhs) == op::Less; },
    /* op::LessEqual */
    [](auto const& lhs, auto const& rhs) -> bool {
        auto const cmp {variant_compare(lhs, rhs)};
        return cmp == op::Less || cmp == op::Equal;
    }};

auto rule::operator()(widget_attribute_types const& other) const -> bool { return op_table[static_cast<usize>(Op)](other, Value); }

auto rule::Equal(widget_attribute_types const& value) -> rule { return {.Op = op::Equal, .Value = value}; }
auto rule::NotEqual(widget_attribute_types const& value) -> rule { return {.Op = op::NotEqual, .Value = value}; }
auto rule::Greater(widget_attribute_types const& value) -> rule { return {.Op = op::Greater, .Value = value}; }
auto rule::GreaterEqual(widget_attribute_types const& value) -> rule { return {.Op = op::GreaterEqual, .Value = value}; }
auto rule::Less(widget_attribute_types const& value) -> rule { return {.Op = op::Less, .Value = value}; }
auto rule::LessEqual(widget_attribute_types const& value) -> rule { return {.Op = op::LessEqual, .Value = value}; }

////////////////////////////////////////////////////////////

style_attributes::style_attributes(std::initializer_list<rules const> values)
{
    for (auto const& val : values) {
        _values[val.first] = val.second;
    }
}

style_attributes::style_attributes(std::span<rules const> values)
{
    for (auto const& val : values) {
        _values[val.first] = val.second;
    }
}

auto style_attributes::score(widget_attributes const& widgetAttribs) const -> i32
{
    if (_values.empty()) { return 0; }

    i32 retValue {0};

    for (auto const& [key, rules] : _values) {
        auto const it {widgetAttribs.find(key)};
        if (it == widgetAttribs.end()) { return FAIL_SCORE; } // widget doesn't have attribute
        auto const& widgetAttrib {it->second};

        for (auto const& rule : rules) {
            if (!rule(widgetAttrib)) { return FAIL_SCORE; }
        }

        ++retValue;
    }

    return retValue;
}

////////////////////////////////////////////////////////////

auto style_flags::score(widget_flags other) const -> i32
{
    i32                                      retValue {0};
    std::array<std::optional<bool>, 5> const flagSet {Focus, Active, Hover, Checked, Disabled};
    std::array<bool, 5> const                otherFlagSet {other.Focus, other.Active, other.Hover, other.Checked, other.Disabled};

    for (usize i {0}; i < flagSet.size(); ++i) {
        if (flagSet[i]) {
            if (*flagSet[i] != otherFlagSet[i]) { // flags don't match
                return FAIL_SCORE;
            }
            ++retValue;
        }
    }
    return retValue;
}

////////////////////////////////////////////////////////////

auto style_collection::get(widget_style_selectors const& select) const -> style*
{
    using score_t = std::tuple<i32, i32>;

    auto it {_styles.find(select.Class)};
    if (it == _styles.end()) { return nullptr; }

    style*  bestCandidate {nullptr};
    score_t bestScore {FAIL_SCORE, FAIL_SCORE};

    for (auto const& [flags, attribs, stylePtr] : it->second) {
        i32 const flagScore {flags.score(select.Flags)};
        if (flagScore == FAIL_SCORE) { continue; }

        i32 const attribScore {attribs.score(select.Attributes)};
        if (attribScore == FAIL_SCORE) { continue; }

        score_t const score {flagScore, attribScore};
        if (score >= bestScore) {
            bestScore     = score;
            bestCandidate = stylePtr.get();
        }
    }

    return bestCandidate;
}

void style_collection::clear()
{
    _styles.clear();
}

}
