// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// IWYU pragma: always_keep

#pragma once
#include "tcob/tcob_config.hpp"

#include <filesystem>
#include <optional>

#include <tuple>
#include <variant>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Concepts.hpp"
#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigTypes.hpp"

#include "tcob/core/ext/magic_enum_reduced.hpp"

namespace tcob::data::config {

////config///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

inline auto converter<object>::IsType(cfg_value const& config) -> bool
{
    return std::holds_alternative<object>(config);
}
inline auto converter<object>::From(cfg_value const& config, object& value) -> bool
{
    if (std::holds_alternative<object>(config)) {
        value = std::get<object>(config);
        return true;
    }
    return false;
}
inline void converter<object>::To(cfg_value& config, object const& value)
{
    config = value;
}

inline auto converter<array>::IsType(cfg_value const& config) -> bool
{
    return std::holds_alternative<array>(config);
}
inline auto converter<array>::From(cfg_value const& config, array& value) -> bool
{
    if (std::holds_alternative<array>(config)) {
        value = std::get<array>(config);
        return true;
    }
    return false;
}
inline void converter<array>::To(cfg_value& config, array const& value)
{
    config = value;
}

template <>
struct converter<cfg_value> {
    auto static IsType(cfg_value const&) -> bool
    {
        return true;
    }

    auto static From(cfg_value const& config, cfg_value& value) -> bool
    {
        value = config;
        return true;
    }

    void static To(cfg_value& config, cfg_value const& value)
    {
        config = value;
    }
};

////basic/////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <>
struct converter<char const*> {
    void static To(cfg_value& config, char const* value)
    {
        config = utf8_string {value};
    }
};

template <usize N>
struct converter<char[N]> { // NOLINT(*-avoid-c-arrays)
    void static To(cfg_value& config, char const* value)
    {
        config = utf8_string {value};
    }
};

template <>
struct converter<string_view> {
    void static To(cfg_value& config, string_view value)
    {
        config = utf8_string {value};
    }
};

template <>
struct converter<utf8_string> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<utf8_string>(config);
    }

    auto static From(cfg_value const& config, utf8_string& value) -> bool
    {
        if (std::holds_alternative<utf8_string>(config)) {
            value = std::get<utf8_string>(config);
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

    void static To(cfg_value& config, utf8_string const& value)
    {
        config = value;
    }
};

template <>
struct converter<bool> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<bool>(config);
    }

    auto static From(cfg_value const& config, bool& value) -> bool
    {
        if (std::holds_alternative<bool>(config)) {
            value = std::get<bool>(config);
            return true;
        }
        return false;
    }

    void static To(cfg_value& config, bool value)
    {
        config = value;
    }
};

template <FloatingPoint T>
struct converter<T> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<f64>(config) || std::holds_alternative<i64>(config);
    }

    auto static From(cfg_value const& config, T& value) -> bool
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

    void static To(cfg_value& config, T const& value)
    {
        config = static_cast<f64>(value);
    }
};

template <Integral T>
struct converter<T> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<i64>(config);
    }

    auto static From(cfg_value const& config, T& value) -> bool
    {
        if (std::holds_alternative<i64>(config)) {
            value = static_cast<T>(std::get<i64>(config));
            return true;
        }
        return false;
    }

    void static To(cfg_value& config, T const& value)
    {
        config = static_cast<i64>(value);
    }
};

template <Enum T>
struct converter<T> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<i64>(config) || std::holds_alternative<utf8_string>(config);
    }

    auto static From(cfg_value const& config, T& value) -> bool
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

    void static To(cfg_value& config, T const& value)
    {
        config = utf8_string {tcob::detail::magic_enum_reduced::enum_to_string(value)};
    }
};

////STL///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename T>
struct converter<std::optional<T>> {
    auto static IsType(cfg_value const& /* config */) -> bool
    {
        return true;
    }

    auto static From(cfg_value const& config, std::optional<T>& value) -> bool
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
};

template <typename... P>
struct converter<std::variant<P...>> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return check_variant<P...>(config);
    }

    auto static From(cfg_value const& config, std::variant<P...>& value) -> bool
    {
        return convert_from<P...>(config, value);
    }

    void static To(cfg_value& config, std::variant<P...> const& value)
    {
        std::visit([&config](auto&& item) { convert_to(config, item); }, value);
    }

private:
    template <typename T, typename... Ts>
    auto static convert_from(cfg_value const& config, std::variant<P...>& value) -> bool
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
    void static convert_to(cfg_value& config, R const& value)
    {
        converter<R>::To(config, value);
    }

    template <typename T, typename... Ts>
    auto static check_variant(cfg_value const& config) -> bool
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
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<object>(config); // TODO: check types
    }

    auto static From(cfg_value const& config, T& value) -> bool
    {
        if (std::holds_alternative<object>(config)) {
            object obj {std::get<object>(config)};
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
        return false;
    }

    void static To(cfg_value& config, T const& value)
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

    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<array>(config); // TODO: check types
    }

    auto static From(cfg_value const& config, T& value) -> bool
    {
        if (std::holds_alternative<array>(config)) {
            array arr {std::get<array>(config)};
            for (auto const& v : arr) {
                auto res {v.template get<key_type>()};
                if (res) {
                    value.insert(res.value());
                } else {
                    return false;
                }
            }

            return true;
        }
        return false;
    }

    void static To(cfg_value& config, T const& value)
    {
        config = array {value};
    }
};

template <typename... T>
struct converter<std::tuple<T...>> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<array>(config); // TODO: check types
    }

    auto static From(cfg_value const& config, std::tuple<T...>& value) -> bool
    {
        if (std::holds_alternative<array>(config)) {
            array arr {std::get<array>(config)};
            std::apply(
                [&arr](auto&&... item) {
                    i32 idx {0};
                    ((from_cfg(arr, idx++, item)), ...);
                },
                value);
            return true;
        }
        return false;
    }

    void static To(cfg_value& config, std::tuple<T...> const& value)
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
    void static from_cfg(array& arr, i32 idx, R& value)
    {
        value = arr[idx].as<R>();
    }

    template <typename R>
    void static to_cfg(array& arr, i32 idx, R& value)
    {
        arr[idx] = value;
    }
};

template <typename K, typename V>
struct converter<std::pair<K, V>> {
    auto static IsType(cfg_value const& config) -> bool
    {
        if (std::holds_alternative<array>(config)) {
            array const arr {std::get<array>(config)};
            return arr.size() >= 2 && arr[0].is<K>() && arr[1].is<V>();
        }

        return false;
    }

    auto static From(cfg_value const& config, std::pair<K, V>& value) -> bool
    {
        if (IsType(config)) {
            array arr {std::get<array>(config)};
            value.first  = arr[0].as<K>();
            value.second = arr[1].as<V>();
            return true;
        }
        return false;
    }

    void static To(cfg_value& config, std::pair<K, V> const& value)
    {
        array arr {value.first, value.second};
        config = arr;
    }
};

template <typename T, usize Size>
struct converter<std::array<T, Size>> {
    auto static IsType(cfg_value const& config) -> bool
    {
        if (std::holds_alternative<array>(config)) {
            array arr {std::get<array>(config)};
            if (arr.size() > 0) {
                return arr.is<T>(0);
            }
            return true;
        }
        return false;
    }

    auto static From(cfg_value const& config, std::array<T, Size>& value) -> bool
    {
        if (std::holds_alternative<array>(config)) {
            array arr {std::get<array>(config)};
            for (isize i {0}; i < arr.size(); ++i) {
                value[static_cast<usize>(i)] = *arr.get<T>(i);
            }
            return true;
        }
        return false;
    }

    void static To(cfg_value& config, std::array<T, Size> const& value)
    {
        array arr {};
        for (auto& val : value) {
            arr.add(val);
        }
        config = arr;
    }
};

template <Container T>
struct converter<T> {
    using value_type = typename T::value_type;

    auto static IsType(cfg_value const& config) -> bool
    {
        if (std::holds_alternative<array>(config)) {
            array arr {std::get<array>(config)};
            if (arr.size() > 0) {
                return arr.is<value_type>(0);
            }
            return true;
        }
        return false;
    }

    auto static From(cfg_value const& config, T& value) -> bool
    {
        if (std::holds_alternative<array>(config)) {
            array arr {std::get<array>(config)};
            for (i32 i {0}; i < arr.size(); ++i) {
                value.push_back(*arr.get<value_type>(i));
            }
            return true;
        }
        return false;
    }

    void static To(cfg_value& config, T const& value)
    {
        array arr {};
        for (auto& val : value) {
            arr.add(val);
        }
        config = arr;
    }
};

template <typename T>
struct converter<std::span<T>> {
    void static To(cfg_value& config, std::span<T> const& value)
    {
        array arr {};
        for (auto const& val : value) {
            arr.add(val);
        }
        config = arr;
    }
};

template <typename T, typename R>
struct converter<std::chrono::duration<T, R>> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return converter<T>::IsType(config);
    }

    auto static From(cfg_value const& config, std::chrono::duration<T, R>& value) -> bool
    {
        T val;
        if (converter<T>::From(config, val)) {
            value = std::chrono::duration<T, R> {val};
            return true;
        }
        return false;
    }

    void static To(cfg_value& config, std::chrono::duration<T, R> const& value)
    {
        converter<T>::To(config, value.count());
    }
};

template <>
struct converter<std::filesystem::path> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<utf8_string>(config);
    }

    auto static From(cfg_value const& config, std::filesystem::path& value) -> bool
    {
        if (std::holds_alternative<utf8_string>(config)) {
            value = std::get<utf8_string>(config);
            return true;
        }

        return false;
    }

    void static To(cfg_value& config, std::filesystem::path const& value)
    {
        config = value.string();
    }
};

////tcob//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <Serializable<object> T>
struct converter<T> {
    auto static IsType(cfg_value const& config) -> bool
    {
        if (std::holds_alternative<object>(config)) {
            T t {};
            return Deserialize(t, std::get<object>(config));
        }
        return false;
    }

    auto static From(cfg_value const& config, T& value) -> bool
    {
        if (std::holds_alternative<object>(config)) {
            return Deserialize(value, std::get<object>(config));
        }
        return false;
    }

    void static To(cfg_value& config, T const& value)
    {
        object obj {};
        Serialize(value, obj);
        config = obj;
    }
};

template <FloatingPoint ValueType, double OneTurn>
struct converter<angle_unit<ValueType, OneTurn>> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return converter<ValueType>::IsType(config);
    }

    auto static From(cfg_value const& config, angle_unit<ValueType, OneTurn>& value) -> bool
    {
        if (IsType(config)) {
            converter<ValueType>::From(config, value.Value);
            return true;
        }

        return false;
    }

    void static To(cfg_value& config, angle_unit<ValueType, OneTurn> const& value)
    {
        config = value.Value;
    }
};

}
