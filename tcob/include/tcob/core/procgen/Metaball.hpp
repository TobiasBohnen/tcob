// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API metaball : public updatable {
public:
    struct ball {
        point_f Position;
        f32     Radius {};
        point_f Velocity;

        auto operator==(ball const&) const -> bool = default;
    };

    explicit metaball(size_i gridSize);

    prop<std::vector<ball>> Balls;

    auto operator()(point_f p) const -> f32;

protected:
    void on_update(milliseconds deltaTime) override;
    auto calculate_field(f32 x, f32 y) const -> f32;

    auto is_dirty() const -> bool;
    void mark_clean();

private:
    size_i _gridSize;
    bool   _dirty {true};
};

}
