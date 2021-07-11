// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <array>
#include <chrono>
#include <memory>
#include <numbers>
#include <string>

namespace tcob {
using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;
using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
using isize = size_t;

using byte = char;
using ubyte = unsigned char;
using sbyte = signed char;

using f32 = float;
static_assert(sizeof(f32) == 4);
using f64 = double;
static_assert(sizeof(f64) == 8);

using ivec2 = std::array<i32, 2>;
using ivec3 = std::array<i32, 3>;
using ivec4 = std::array<i32, 4>;
using uvec2 = std::array<u32, 2>;
using uvec3 = std::array<u32, 3>;
using uvec4 = std::array<u32, 4>;
using vec2 = std::array<f32, 2>;
using vec3 = std::array<f32, 3>;
using vec4 = std::array<f32, 4>;

using mat2x3 = std::array<f32, 2 * 3>;
using mat3x4 = std::array<f32, 3 * 4>;
using mat4 = std::array<f32, 4 * 4>;
using mat3 = std::array<f32, 3 * 3>;

constexpr auto TAU { std::numbers::pi * 2 };
constexpr auto TAU_F { std::numbers::pi_v<f32> * 2 };

using MilliSeconds = std::chrono::duration<double, std::milli>;

//forward decl
class Game;

class ResourceGroup;
template <typename T>
class Resource;
template <typename T>
class ResourceLoader;
class ResourceLibrary;

namespace lua {
    template <typename T>
    struct Converter;
}

}