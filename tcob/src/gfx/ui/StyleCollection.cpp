// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/StyleCollection.hpp"

#include <array>
#include <initializer_list>
#include <limits>
#include <optional>
#include <utility>

#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

style_attributes::style_attributes(std::initializer_list<std::pair<string, widget_attribute_types> const> values)
{
    for (auto const& val : values) {
        _values[val.first].insert(val.second);
    }
}

auto style_attributes::score(widget_attributes const& widgetAttribs) const -> i32
{
    if (_values.empty()) { return 0; }

    i32 retValue {0};
    for (auto const& [key, requiredValues] : _values) {
        if (!widgetAttribs.contains(key)) { return std::numeric_limits<i32>::min(); }                    // widget doesn't have attribute
        if (!requiredValues.contains(widgetAttribs.at(key))) { return std::numeric_limits<i32>::min(); } // attribute values don't match
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
                return std::numeric_limits<i32>::min();
            }
            ++retValue;
        }
    }
    return retValue;
}

////////////////////////////////////////////////////////////

auto style_collection::get(widget_style_selectors const& select) const -> style*
{
    auto it {_styles.find(select.Class)};
    if (it == _styles.end()) { return nullptr; }

    style* bestCandidate {nullptr};
    i32    bestScore {std::numeric_limits<i32>::min()};

    for (auto const& [flags, attribs, stylePtr] : it->second) {
        i32 const attibScore {attribs.score(select.Attributes)};
        if (attibScore == std::numeric_limits<i32>::min()) { continue; }
        i32 const flagScore {flags.score(select.Flags)};
        if (flagScore == std::numeric_limits<i32>::min()) { continue; }
        i32 const score {(flagScore * 10000) + attibScore};

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
