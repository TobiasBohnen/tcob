// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
alignment const alignment::TopLeft {.Horizontal = horizontal_alignment::Left, .Vertical = vertical_alignment::Top};
alignment const alignment::TopCenter {.Horizontal = horizontal_alignment::Center, .Vertical = vertical_alignment::Top};
alignment const alignment::TopRight {.Horizontal = horizontal_alignment::Right, .Vertical = vertical_alignment::Top};
alignment const alignment::MiddleLeft {.Horizontal = horizontal_alignment::Left, .Vertical = vertical_alignment::Middle};
alignment const alignment::MiddleCenter {.Horizontal = horizontal_alignment::Center, .Vertical = vertical_alignment::Middle};
alignment const alignment::MiddleRight {.Horizontal = horizontal_alignment::Right, .Vertical = vertical_alignment::Middle};
alignment const alignment::BottomLeft {.Horizontal = horizontal_alignment::Left, .Vertical = vertical_alignment::Bottom};
alignment const alignment::BottomCenter {.Horizontal = horizontal_alignment::Center, .Vertical = vertical_alignment::Bottom};
alignment const alignment::BottomRight {.Horizontal = horizontal_alignment::Right, .Vertical = vertical_alignment::Bottom};
}
