// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <initializer_list>
#include <memory>
#include <optional>
#include <span>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

enum class op : u8 {
    Equal        = 0,
    NotEqual     = 1,
    Greater      = 2,
    GreaterEqual = 3,
    Less         = 4,
    LessEqual    = 5
};

class TCOB_API rule final {
public:
    op                     Op {};
    widget_attribute_types Value;

    auto operator()(widget_attribute_types const& other) const -> bool;

    auto operator==(rule const& other) const -> bool = default;

    static auto Equal(widget_attribute_types const& value) -> rule;
    static auto NotEqual(widget_attribute_types const& value) -> rule;
    static auto Greater(widget_attribute_types const& value) -> rule;
    static auto GreaterEqual(widget_attribute_types const& value) -> rule;
    static auto Less(widget_attribute_types const& value) -> rule;
    static auto LessEqual(widget_attribute_types const& value) -> rule;
};

////////////////////////////////////////////////////////////

class TCOB_API style_attributes final {
public:
    using rules = std::pair<string, std::vector<rule>>;

    style_attributes() = default;
    style_attributes(std::initializer_list<rules const> values);
    explicit style_attributes(std::span<rules const> values);

    auto score(widget_attributes const& widgetAttribs) const -> i32;

    auto operator==(style_attributes const& other) const -> bool = default;

private:
    std::unordered_map<string, std::vector<rule>> _values;
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
