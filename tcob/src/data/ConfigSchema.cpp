// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/ConfigSchema.hpp"

#include "tcob/core/StringUtils.hpp"

namespace tcob::data::config {

auto schema::validate(object const& obj) const -> result
{
    result retValue {};
    auto   valid {[&](auto&& arg) { return validate_property(obj, arg); }};

    if (!AllOf.empty()) {
        for (auto const& e : AllOf) {
            if (auto fail {std::visit(valid, e)}) {
                fail->Group = "AllOf";
                retValue.Failures.push_back(*fail);
            }
        }
    }

    if (!AnyOf.empty()) {
        bool                 test {false};
        std::vector<failure> failures;
        for (auto const& e : AnyOf) {
            if (auto fail {std::visit(valid, e)}) {
                fail->Group = "AnyOf";
                failures.push_back(*fail);
            } else {
                test = true;
            }
        }

        if (!test) {
            retValue.Failures.insert(retValue.Failures.end(), failures.begin(), failures.end());
        }
    }

    if (!OneOf.empty()) {
        bool                 test {false};
        std::vector<failure> failures;
        for (auto const& e : OneOf) {
            if (auto fail {std::visit(valid, e)}) {
                fail->Group = "OneOf";
                failures.push_back(*fail);
            } else {
                if (test) {
                    retValue.Failures.push_back({.Group      = "OneOf",
                                                 .Name       = std::visit([](auto const& a) { return a.Name; }, e),
                                                 .Constraint = "Group"});
                }
                test = true;
            }
        }

        if (!test) {
            retValue.Failures.insert(retValue.Failures.end(), failures.begin(), failures.end());
        }
    }

    if (!NoneOf.empty()) {
        for (auto const& e : NoneOf) {
            if (!std::visit(valid, e)) {
                retValue.Failures.push_back({.Group      = "NoneOf",
                                             .Name       = std::visit([](auto const& a) { return a.Name; }, e),
                                             .Constraint = "Group"});
            }
        }
    }

    return retValue;
}

auto schema::FromObject(object const& obj) -> std::shared_ptr<schema>
{
    auto retValue {std::make_shared<schema>()};

    if (object propsSec; obj.try_get(propsSec, "Properties")) {
        flat_map<string, property> props;

        for (auto const& [k, v] : propsSec) {
            if (object propSec; v.try_get(propSec)) {
                if (type propType {}; propSec.try_get(propType, "Type")) {
                    property prop;

                    switch (propType) {
                    case type::String: {
                        string_property val;
                        val.Name = k;
                        if (usize max {0}; propSec.try_get(max, "MaxLength")) {
                            val.MaxLength = max;
                        }
                        if (usize min {0}; propSec.try_get(min, "MinLength")) {
                            val.MinLength = min;
                        }
                        propSec.try_get(val.Pattern, "Pattern");
                        prop = val;
                    } break;
                    case type::Float: {
                        float_property val;
                        val.Name = k;
                        if (f64 max {0}; propSec.try_get(max, "MaxValue")) {
                            val.MaxValue = max;
                        }
                        if (f64 min {0}; propSec.try_get(min, "MinValue")) {
                            val.MinValue = min;
                        }
                        prop = val;
                    } break;
                    case type::Integer: {
                        int_property val;
                        val.Name = k;
                        if (i64 max {0}; propSec.try_get(max, "MaxValue")) {
                            val.MaxValue = max;
                        }
                        if (i64 min {0}; propSec.try_get(min, "MinValue")) {
                            val.MinValue = min;
                        }
                        prop = val;
                    } break;
                    case type::Bool: {
                        bool_property val;
                        val.Name = k;
                        prop     = val;
                    } break;
                    case type::Array: {
                        array_property val;
                        val.Name = k;
                        if (isize max {0}; propSec.try_get(max, "MaxSize")) {
                            val.MaxSize = max;
                        }
                        if (isize min {0}; propSec.try_get(min, "MinSize")) {
                            val.MinSize = min;
                        }
                        if (type itype {}; propSec.try_get(itype, "Type")) {
                            val.ItemType = itype;
                        }
                        prop = val;
                    } break;
                    case type::Object: {
                        object_property val;
                        val.Name = k;
                        if (string subSchemaName; propSec.try_get(subSchemaName, "Schema")) {
                            if (object subSchema; obj.try_get(subSchema, "Schemas", subSchemaName)) {
                                val.Schema = schema::FromObject(subSchema);
                            }
                        }
                        prop = val;
                    } break;
                    case type::Null: break;
                    }

                    props[k] = prop;
                }
            }
        }

        if (array allOf; obj.try_get(allOf, "AllOf")) {
            for (auto const& item : allOf) {
                retValue->AllOf.push_back(props[item.as<string>()]);
            }
        }
        if (array anyOf; obj.try_get(anyOf, "AnyOf")) {
            for (auto const& item : anyOf) {
                retValue->AnyOf.push_back(props[item.as<string>()]);
            }
        }
        if (array oneOf; obj.try_get(oneOf, "OneOf")) {
            for (auto const& item : oneOf) {
                retValue->OneOf.push_back(props[item.as<string>()]);
            }
        }
        if (array noneOf; obj.try_get(noneOf, "NoneOf")) {
            for (auto const& item : noneOf) {
                retValue->NoneOf.push_back(props[item.as<string>()]);
            }
        }
    }

    return retValue;
}

auto schema::validate_property(object const& obj, string_property const& prop) const -> std::optional<failure>
{
    failure retValue {.Group = "", .Name = prop.Name, .Constraint = ""};

    if (!obj.has(prop.Name)) {
        retValue.Constraint = "Name";
    } else if (!obj.is<string>(prop.Name)) {
        retValue.Constraint = "Type";
    } else {
        auto const val {obj.as<string>(prop.Name)};
        if (prop.MaxLength && val.length() > *prop.MaxLength) {
            retValue.Constraint = "MaxLength";
        } else if (prop.MinLength && val.length() < *prop.MinLength) {
            retValue.Constraint = "MinLength";
        } else if (!prop.Pattern.empty() && !helper::wildcard_match(val, prop.Pattern)) {
            retValue.Constraint = "Pattern";
        }
    }

    return retValue.Constraint.empty() ? std::nullopt : std::optional {retValue};
}

auto schema::validate_property(object const& obj, float_property const& prop) const -> std::optional<failure>
{
    failure retValue {.Group = "", .Name = prop.Name, .Constraint = ""};

    if (!obj.has(prop.Name)) {
        retValue.Constraint = "Name";
    } else {
        if (!obj.is<f64>(prop.Name)) {
            retValue.Constraint = "Type";
        } else {
            auto const val {obj.as<f64>(prop.Name)};
            if (prop.MaxValue && val > *prop.MaxValue) {
                retValue.Constraint = "MaxValue";
            } else if (prop.MinValue && val < *prop.MinValue) {
                retValue.Constraint = "MinValue";
            }
        }
    }

    return retValue.Constraint.empty() ? std::nullopt : std::optional {retValue};
}

auto schema::validate_property(object const& obj, int_property const& prop) const -> std::optional<failure>
{
    failure retValue {.Group = "", .Name = prop.Name, .Constraint = ""};

    if (!obj.has(prop.Name)) {
        retValue.Constraint = "Name";
    } else {
        if (!obj.is<i64>(prop.Name)) {
            retValue.Constraint = "Type";
        } else {
            auto const val {obj.as<i64>(prop.Name)};
            if (prop.MaxValue && val > *prop.MaxValue) {
                retValue.Constraint = "MaxValue";
            } else if (prop.MinValue && val < *prop.MinValue) {
                retValue.Constraint = "MinValue";
            }
        }
    }

    return retValue.Constraint.empty() ? std::nullopt : std::optional {retValue};
}

auto schema::validate_property(object const& obj, bool_property const& prop) const -> std::optional<failure>
{
    failure retValue {.Group = "", .Name = prop.Name, .Constraint = ""};

    if (!obj.has(prop.Name)) {
        retValue.Constraint = "Name";
    } else if (!obj.is<bool>(prop.Name)) {
        retValue.Constraint = "Type";
    }

    return retValue.Constraint.empty() ? std::nullopt : std::optional {retValue};
}

auto schema::validate_property(object const& obj, array_property const& prop) const -> std::optional<failure>
{
    failure retValue {.Group = "", .Name = prop.Name, .Constraint = ""};

    if (!obj.has(prop.Name)) {
        retValue.Constraint = "Name";
    } else {
        if (!obj.is<array>(prop.Name)) {
            retValue.Constraint = "Type";
        } else {
            auto const val {obj.as<array>(prop.Name)};
            if (prop.MaxSize && val.get_size() > *prop.MaxSize) {
                retValue.Constraint = "MaxSize";
            } else if (prop.MinSize && val.get_size() < *prop.MinSize) {
                retValue.Constraint = "MinSize";
            } else if (prop.ItemType) {
                for (isize i {0}; i < val.get_size(); ++i) {
                    bool test {false};
                    switch (*prop.ItemType) {
                    case type::Array:
                        test = val.is<array>(i);
                        break;
                    case type::Bool:
                        test = val.is<bool>(i);
                        break;
                    case type::Float:
                        test = val.is<f64>(i);
                        break;
                    case type::Integer:
                        test = val.is<i64>(i);
                        break;
                    case type::Object:
                        test = val.is<object>(i);
                        break;
                    case type::String:
                        test = val.is<string>(i);
                        break;
                    case type::Null: break;
                    }
                    if (!test) { retValue.Constraint = "ItemType"; }
                }
            }
        }
    }

    return retValue.Constraint.empty() ? std::nullopt : std::optional {retValue};
}

auto schema::validate_property(object const& obj, object_property const& prop) const -> std::optional<failure>
{
    failure retValue {.Group = "", .Name = prop.Name, .Constraint = ""};

    if (!obj.has(prop.Name)) {
        retValue.Constraint = "Name";
    } else {
        if (!obj.is<object>(prop.Name)) {
            retValue.Constraint = "Type";
        } else if (prop.Schema && !prop.Schema->validate(obj.as<object>(prop.Name))) {
            retValue.Constraint = "Schema";
        }
    }

    return retValue.Constraint.empty() ? std::nullopt : std::optional {retValue};
}

////////////////////////////////////////////////////////////

schema::result::operator bool() const
{
    return Failures.empty();
}

} // namespace config
