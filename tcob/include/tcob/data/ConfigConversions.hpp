// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// IWYU pragma: always_keep

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <chrono>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <variant>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Concepts.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigTypes.hpp"

#include "tcob/core/ext/magic_enum_reduced.hpp"

namespace tcob::data {

////config///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <>
struct converter<cfg_value> {
    static auto IsType(cfg_value const&) -> bool
    {
        return true;
    }

    static auto From(cfg_value const& config, cfg_value& value) -> bool
    {
        value = config;
        return true;
    }

    static void To(cfg_value& config, cfg_value const& value)
    {
        config = value;
    }
};

////basic/////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <>
struct converter<char const*> {
    static void To(cfg_value& config, char const* value)
    {
        config = utf8_string {value};
    }
};

template <usize N>
struct converter<char[N]> { // NOLINT(*-avoid-c-arrays)
    static void To(cfg_value& config, char const* value)
    {
        config = utf8_string {value};
    }
};

template <>
struct converter<string_view> {
    static void To(cfg_value& config, string_view value)
    {
        config = utf8_string {value};
    }
};

template <>
struct converter<string> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<string>(config);
    }

    static auto From(cfg_value const& config, string& value) -> bool
    {
        if (std::holds_alternative<string>(config)) {
            value = std::get<string>(config);
            return true;
        }

        if (std::holds_alternative<i64>(config)) {
            value = std::to_string(std::get<i64>(config));
            return true;
        }

        if (std::holds_alternative<f64>(config)) {
            value = std::to_string(std::get<f64>(config));
            return true;
        }

        if (std::holds_alternative<bool>(config)) {
            value = std::get<bool>(config) ? "true" : "false";
            return true;
        }

        if (std::holds_alternative<array>(config)) {
            value = std::get<array>(config).str();
            return true;
        }

        if (std::holds_alternative<object>(config)) {
            value = std::get<object>(config).str();
            return true;
        }
        return false;
    }

    static void To(cfg_value& config, string const& value)
    {
        config = value;
    }
};

template <>
struct converter<bool> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<bool>(config);
    }

    static auto From(cfg_value const& config, bool& value) -> bool
    {
        if (std::holds_alternative<bool>(config)) {
            value = std::get<bool>(config);
            return true;
        }
        return false;
    }

    static void To(cfg_value& config, bool value)
    {
        config = value;
    }
};

template <FloatingPoint T>
struct converter<T> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<f64>(config) || std::holds_alternative<i64>(config);
    }

    static auto From(cfg_value const& config, T& value) -> bool
    {
        if (std::holds_alternative<f64>(config)) {
            value = static_cast<T>(std::get<f64>(config));
            return true;
        }

        if (std::holds_alternative<i64>(config)) {
            value = static_cast<T>(std::get<i64>(config));
            return true;
        }

        return false;
    }

    static void To(cfg_value& config, T const& value)
    {
        config = static_cast<f64>(value);
    }
};

template <Integral T>
struct converter<T> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<i64>(config);
    }

    static auto From(cfg_value const& config, T& value) -> bool
    {
        if (std::holds_alternative<i64>(config)) {
            value = static_cast<T>(std::get<i64>(config));
            return true;
        }
        return false;
    }

    static void To(cfg_value& config, T const& value)
    {
        config = static_cast<i64>(value);
    }
};

template <Enum T>
struct converter<T> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<i64>(config) || std::holds_alternative<utf8_string>(config);
    }

    static auto From(cfg_value const& config, T& value) -> bool
    {
        if (std::holds_alternative<utf8_string>(config)) {
            value = tcob::detail::magic_enum_reduced::string_to_enum<T>(std::get<utf8_string>(config));
            return true;
        }

        if (std::holds_alternative<i64>(config)) {
            value = static_cast<T>(std::get<i64>(config));
            return true;
        }

        return false;
    }

    static void To(cfg_value& config, T const& value)
    {
        config = utf8_string {tcob::detail::magic_enum_reduced::enum_to_string(value)};
    }
};

////STL///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename T>
struct converter<std::optional<T>> {
    static auto IsType(cfg_value const& /* config */) -> bool
    {
        return true;
    }

    static auto From(cfg_value const& config, std::optional<T>& value) -> bool
    {
        if (!converter<T>::IsType(config)) {
            value = std::nullopt;
        } else {
            T val {};
            converter<T>::From(config, val);
            value = val;
        }

        return true;
    }

    static void To(cfg_value& config, std::optional<T> const& value)
    {
        if (value) {
            converter<T>::To(config, *value);
        } else {
            config = std::monostate {};
        }
    }
};

template <>
struct converter<std::nullopt_t> {
    static void To(cfg_value& config, std::nullopt_t const&)
    {
        config = std::monostate {};
    }
};

template <typename... P>
struct converter<std::variant<P...>> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return check_variant<P...>(config);
    }

    static auto From(cfg_value const& config, std::variant<P...>& value) -> bool
    {
        return convert_from<P...>(config, value);
    }

    static void To(cfg_value& config, std::variant<P...> const& value)
    {
        std::visit([&config](auto&& item) { convert_to(config, item); }, value);
    }

private:
    template <typename T, typename... Ts>
    static auto convert_from(cfg_value const& config, std::variant<P...>& value) -> bool
    {
        if (converter<T>::IsType(config)) {
            T val {};
            converter<T>::From(config, val);
            value = val;
            return true;
        }

        if constexpr (sizeof...(Ts) > 0) {
            return convert_from<Ts...>(config, value);
        } else {
            return false;
        }
    }

    template <typename R>
    static void convert_to(cfg_value& config, R const& value)
    {
        converter<R>::To(config, value);
    }

    template <typename T, typename... Ts>
    static auto check_variant(cfg_value const& config) -> bool
    {
        if constexpr (sizeof...(Ts) > 0) {
            return converter<T>::IsType(config) || check_variant<Ts...>(config);
        } else {
            return converter<T>::IsType(config);
        }
    }
};

template <Map T>
struct converter<T> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<object>(config); // TODO: check types
    }

    static auto From(cfg_value const& config, T& value) -> bool
    {
        if (!std::holds_alternative<object>(config)) { return false; }

        object obj {std::get<object>(config)};
        value.clear();

        for (auto const& [k, v] : obj) {
            auto res {v.template get<typename T::mapped_type>()};
            if (res) {
                value[k] = res.value();
            } else {
                return false;
            }
        }

        return true;
    }

    static void To(cfg_value& config, T const& value)
    {
        object obj {};
        for (auto const& [key, val] : value) {
            obj[key] = val;
        }
        config = obj;
    }
};

template <Set T>
struct converter<T> {
    using key_type = typename T::key_type;

    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<array>(config); // TODO: check types
    }

    static auto From(cfg_value const& config, T& value) -> bool
    {
        if (!std::holds_alternative<array>(config)) { return false; }

        value.clear();
        array arr {std::get<array>(config)};
        for (auto const& v : arr) {
            if (auto res {v.template get<key_type>()}) {
                value.insert(res.value());
            } else {
                return false;
            }
        }

        return true;
    }

    static void To(cfg_value& config, T const& value)
    {
        config = array {value};
    }
};

template <typename... T>
struct converter<std::tuple<T...>> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<array>(config); // TODO: check types
    }

    static auto From(cfg_value const& config, std::tuple<T...>& value) -> bool
    {
        if (!std::holds_alternative<array>(config)) { return false; }

        array arr {std::get<array>(config)};
        std::apply(
            [&arr](auto&&... item) {
                i32 idx {0};
                ((from_cfg(arr, idx++, item)), ...);
            },
            value);
        return true;
    }

    static void To(cfg_value& config, std::tuple<T...> const& value)
    {
        array arr {};
        std::apply(
            [&arr](auto&&... item) {
                i32 idx {0};
                ((to_cfg(arr, idx++, item)), ...);
            },
            value);

        config = arr;
    }

private:
    template <typename R>
    static void from_cfg(array& arr, i32 idx, R& value)
    {
        value = arr[idx].as<R>();
    }

    template <typename R>
    static void to_cfg(array& arr, i32 idx, R& value)
    {
        arr[idx] = value;
    }
};

template <typename K, typename V>
struct converter<std::pair<K, V>> {
    static auto IsType(cfg_value const& config) -> bool
    {
        if (std::holds_alternative<array>(config)) {
            array const arr {std::get<array>(config)};
            return arr.size() >= 2 && arr[0].is<K>() && arr[1].is<V>();
        }

        return false;
    }

    static auto From(cfg_value const& config, std::pair<K, V>& value) -> bool
    {
        if (IsType(config)) {
            array arr {std::get<array>(config)};
            value.first  = arr[0].as<K>();
            value.second = arr[1].as<V>();
            return true;
        }
        return false;
    }

    static void To(cfg_value& config, std::pair<K, V> const& value)
    {
        array arr {value.first, value.second};
        config = arr;
    }
};

template <typename T, usize Size>
struct converter<std::array<T, Size>> {
    static auto IsType(cfg_value const& config) -> bool
    {
        if (!std::holds_alternative<array>(config)) { return false; }

        array arr {std::get<array>(config)};
        if (arr.size() > 0) { return arr.is<T>(0); }
        return true;
    }

    static auto From(cfg_value const& config, std::array<T, Size>& value) -> bool
    {
        if (!std::holds_alternative<array>(config)) { return false; }

        array arr {std::get<array>(config)};
        for (isize i {0}; i < arr.size(); ++i) {
            value[static_cast<usize>(i)] = *arr.get<T>(i);
        }
        return true;
    }

    static void To(cfg_value& config, std::array<T, Size> const& value)
    {
        array arr {};
        for (auto& val : value) { arr.add(val); }
        config = arr;
    }
};

template <Container T>
struct converter<T> {
    using value_type = typename T::value_type;

    static auto IsType(cfg_value const& config) -> bool
    {
        if (!std::holds_alternative<array>(config)) { return false; }

        array arr {std::get<array>(config)};
        if (arr.size() > 0) { return arr.is<value_type>(0); }
        return true;
    }

    static auto From(cfg_value const& config, T& value) -> bool
    {
        if (!std::holds_alternative<array>(config)) { return false; }

        value.clear();
        array arr {std::get<array>(config)};
        for (i32 i {0}; i < arr.size(); ++i) {
            value.push_back(*arr.get<value_type>(i));
        }
        return true;
    }

    static void To(cfg_value& config, T const& value)
    {
        array arr {};
        for (auto& val : value) { arr.add(val); }
        config = arr;
    }
};

template <typename T>
struct converter<std::span<T>> {
    static void To(cfg_value& config, std::span<T> const& value)
    {
        array arr {};
        for (auto const& val : value) { arr.add(val); }
        config = arr;
    }
};

template <typename T, typename R>
struct converter<std::chrono::duration<T, R>> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return converter<T>::IsType(config);
    }

    static auto From(cfg_value const& config, std::chrono::duration<T, R>& value) -> bool
    {
        T val;
        if (converter<T>::From(config, val)) {
            value = std::chrono::duration<T, R> {val};
            return true;
        }
        return false;
    }

    static void To(cfg_value& config, std::chrono::duration<T, R> const& value)
    {
        converter<T>::To(config, value.count());
    }
};

template <>
struct converter<std::filesystem::path> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<utf8_string>(config);
    }

    static auto From(cfg_value const& config, std::filesystem::path& value) -> bool
    {
        if (std::holds_alternative<utf8_string>(config)) {
            value = std::get<utf8_string>(config);
            return true;
        }

        return false;
    }

    static void To(cfg_value& config, std::filesystem::path const& value)
    {
        config = value.string();
    }
};

template <>
struct converter<std::monostate> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<std::monostate>(config);
    }

    static auto From(cfg_value const& config, std::monostate& /* value */) -> bool
    {
        return std::holds_alternative<std::monostate>(config);
    }

    static void To(cfg_value& config, std::monostate const& value)
    {
        config = value;
    }
};

////tcob//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <Serializable T>
struct converter<T> {
public:
    static auto IsType(cfg_value const& config) -> bool
    {
        if (std::holds_alternative<object>(config)) {
            T t {};
            return From(config, t);
        }
        return false;
    }

    static auto From(cfg_value const& config, T& value) -> bool
    {
        if (!std::holds_alternative<object>(config)) { return false; }

        object const&     obj {std::get<object>(config)};
        static auto const members {T::Members()};
        bool              retValue {true};
        std::apply([&](auto&&... m) {
            ((retValue = retValue && m.set(obj, value)), ...);
        },
                   members);
        return retValue;
    }

    static void To(cfg_value& config, T const& value)
    {
        object obj {};

        static auto const members {T::Members()};
        std::apply([&](auto&&... m) { ((m.get(obj, value)), ...); },
                   members);

        config = obj;
    }
};

template <FloatingPoint ValueType, double OneTurn>
struct converter<angle_unit<ValueType, OneTurn>> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return converter<ValueType>::IsType(config);
    }

    static auto From(cfg_value const& config, angle_unit<ValueType, OneTurn>& value) -> bool
    {
        if (IsType(config)) {
            converter<ValueType>::From(config, value.Value);
            return true;
        }

        return false;
    }

    static void To(cfg_value& config, angle_unit<ValueType, OneTurn> const& value)
    {
        config = value.Value;
    }
};

template <typename T>
struct converter<prop<T>> {
    static auto IsType(cfg_value const& config) -> bool
    {
        return converter<T>::IsType(config);
    }

    static auto From(cfg_value const& config, prop<T>& value) -> bool
    {
        if (IsType(config)) {
            if (T val; converter<T>::From(config, val)) {
                value = val;
                return true;
            }
        }

        return false;
    }

    static void To(cfg_value& config, prop<T> const& value)
    {
        converter<T>::To(config, value);
    }
};

}
