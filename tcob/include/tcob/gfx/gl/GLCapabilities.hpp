// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

namespace tcob::gl::Capabilities {

inline std::pair<f32, f32> PointSizeRange;
inline f32 PointSizeGranularity;
inline i32 MaxTextureSize;
inline i32 MaxArrayTextureLayers;

void init();
}