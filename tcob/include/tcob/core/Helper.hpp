// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <concepts>
#include <sstream>

namespace tcob {
namespace detail {
    //from https://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
    template <typename Test, template <typename...> typename Ref>
    struct is_specialization : std::false_type {
    };
    template <template <typename...> typename Ref, typename... Args>
    struct is_specialization<Ref<Args...>, Ref> : std::true_type {
    };
}

template <typename T>
concept Boolean = std::is_same_v<T, bool>;

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept Integral = std::is_integral_v<T>;

template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

template <typename T>
concept Enum = std::is_enum_v<T>;

template <typename T>
concept Pointer = std::is_pointer_v<T>;

}