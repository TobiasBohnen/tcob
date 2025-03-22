// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <initializer_list>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

class TCOB_API style_attributes final {
public:
    style_attributes() = default;
    style_attributes(std::initializer_list<std::pair<string, widget_attribute_types> const> values);

    auto score(widget_attributes const& widgetAttribs) const -> i32;

    auto operator==(style_attributes const& other) const -> bool = default;

private:
    std::unordered_map<string, std::unordered_set<widget_attribute_types>> _values;
};

////////////////////////////////////////////////////////////

class TCOB_API style_flags final {
public:
    std::optional<bool> Focus {};
    std::optional<bool> Active {};
    std::optional<bool> Hover {};
    std::optional<bool> Checked {};
    std::optional<bool> Disabled {};

    auto score(widget_flags other) const -> i32;

    auto operator==(style_flags const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API style_collection final {
public:
    template <std::derived_from<style> T>
    auto create(string const& name, style_flags flags, style_attributes const& attribs = {}) -> std::shared_ptr<T>;

    template <std::derived_from<widget> T>
    auto create(string const& name, style_flags flags, style_attributes const& attribs = {}) -> std::shared_ptr<typename T::style>;

    auto get(widget_style_selectors const& select) const -> style*;

    void clear();

private:
    std::unordered_map<string, std::vector<std::tuple<style_flags, style_attributes, std::shared_ptr<style>>>> _styles;
};

}

#include "StyleCollection.inl"
