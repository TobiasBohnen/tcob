// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

namespace tcob::data::sqlite::detail {
////////////////////////////////////////////////////////////

auto register_vfs() -> string;

}

#endif
