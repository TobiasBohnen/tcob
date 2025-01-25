// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::config {
////////////////////////////////////////////////////////////

class TCOB_API schema {
public:
    struct failure {
        string Group;
        string Name;
        string Constraint;
    };
    class TCOB_API result {
    public:
        std::vector<failure> Failures;

        explicit operator bool() const;
    };

    struct string_property {
        string               Name {};
        std::optional<usize> MinLength {};
        std::optional<usize> MaxLength {};
        string               Pattern {};
    };
    struct float_property {
        string             Name {};
        std::optional<f64> MinValue {};
        std::optional<f64> MaxValue {};
    };
    struct int_property {
        string             Name {};
        std::optional<i64> MinValue {};
        std::optional<i64> MaxValue {};
    };
    struct bool_property {
        string Name;
    };
    struct array_property {
        string               Name {};
        std::optional<isize> MinSize {};
        std::optional<isize> MaxSize {};
        std::optional<type>  ItemType {};
    };
    struct object_property {
        string                  Name {};
        std::shared_ptr<schema> Schema {};
    };
    using property = std::variant<string_property, float_property, int_property, bool_property, array_property, object_property>;

    std::vector<property> AllOf;
    std::vector<property> AnyOf;
    std::vector<property> OneOf;
    std::vector<property> NoneOf;

    auto validate(object const& obj) const -> result;

    auto static FromObject(object const& obj) -> std::shared_ptr<schema>;

private:
    auto validate_property(object const& obj, string_property const& prop) const -> std::optional<failure>;
    auto validate_property(object const& obj, float_property const& prop) const -> std::optional<failure>;
    auto validate_property(object const& obj, int_property const& prop) const -> std::optional<failure>;
    auto validate_property(object const& obj, bool_property const& prop) const -> std::optional<failure>;
    auto validate_property(object const& obj, array_property const& prop) const -> std::optional<failure>;
    auto validate_property(object const& obj, object_property const& prop) const -> std::optional<failure>;
};
}
