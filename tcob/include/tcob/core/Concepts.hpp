// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// IWYU pragma: always_keep

#pragma once
#include "tcob/tcob_config.hpp"

#include <concepts>
#include <iterator>
#include <span>
#include <type_traits>

////////////////////////////////////////////////////////////
namespace tcob {

template <typename T>
concept Boolean = std::is_same_v<T, bool>;

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept Integral = std::is_integral_v<T>;

template <typename T>
concept Unsigned = std::is_unsigned_v<T> && std::is_integral_v<T>;

template <typename T>
concept Signed = std::is_signed_v<T> && std::is_integral_v<T>;

template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

template <typename T>
concept Enum = std::is_enum_v<T>;

template <typename T>
concept Pointer = std::is_pointer_v<T>;

template <typename T>
concept Reference = std::is_reference_v<T>;

template <typename T, typename D>
concept BaseOf = std::is_base_of_v<T, D> && std::is_convertible_v<D const*, T const*>;

template <typename T, typename D>
concept DerivedFrom = std::derived_from<D, T>;

template <typename T, typename D>
concept BaseOfOrDerivedFrom = BaseOf<T, D> || DerivedFrom<D, T>;

template <typename T>
concept POD = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

template <typename T>
concept Equatable = requires(T& t) { { t == t }; };

template <typename T>
concept LessComparable = requires(T& t) { { t < t }; };

template <typename T>
concept LessEqualComparable = requires(T& t) { { t <= t }; };

template <typename T>
concept Negatable = requires(T& t) { { -t }; };

template <typename T>
concept Addable = requires(T& t) { { t + t }; };

template <typename T>
concept Subtractable = requires(T& t) { { t - t }; };

template <typename T>
concept Multipliable = requires(T& t) { { t* t }; };

template <typename T>
concept Dividable = requires(T& t) { { t / t }; };

template <typename T>
concept HasSize = requires(T& t) { { t.size() }; };

template <typename T>
concept OverloadsArrowOp = requires(T a) { { a.operator->() } -> Pointer; };

template <typename T>
concept Serializable =
    requires(T&) {
        { T::Members() };
    };

template <typename T>
concept Map = requires(T a, typename T::key_type k, typename T::mapped_type v) {
    typename T::key_type;
    typename T::mapped_type;
    { a.clear() };
    { a[k] } -> std::convertible_to<typename T::mapped_type&>;
    { a.begin() } -> std::forward_iterator;
    { a.end() } -> std::sentinel_for<decltype(a.begin())>;
};

template <typename T>
concept Set = requires(T a, typename T::key_type key) {
    typename T::key_type;
    { a.insert(key) };
    { a.clear() };
    { a.size() } -> std::convertible_to<usize>;
    { a.begin() } -> std::forward_iterator;
    { a.end() } -> std::sentinel_for<decltype(a.begin())>;
};

template <typename T>
concept StringLike = std::is_convertible_v<T, string_view>;

template <typename T>
concept NotStringLikePOD = POD<T> && !StringLike<T> && !std::is_same_v<T, std::span<std::remove_const_t<T>>>;

template <typename T>
concept Container =
    requires(T& container, typename T::value_type& value) {
        typename T::value_type;

        { container.push_back(value) };
        { container.clear() };
        { container.resize(usize {}) };
        { container.size() } -> std::convertible_to<usize>;
        { container.operator[](usize {}) };
    } && !StringLike<T>;

}
