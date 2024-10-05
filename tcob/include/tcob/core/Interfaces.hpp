// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

enum class update_mode : u8 {
    Normal,
    Fixed,
    Both
};

class TCOB_API updatable {
public:
    virtual ~updatable() = default;

    void virtual update(milliseconds deltaTime);

protected:
    updatable()                                                   = default;
    updatable(updatable const& other) noexcept                    = default;
    auto operator=(updatable const& other) noexcept -> updatable& = default;
    updatable(updatable&& other) noexcept                         = default;
    auto operator=(updatable&& other) noexcept -> updatable&      = default;

    void virtual on_update(milliseconds deltaTime) = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API non_copyable {
protected:
    non_copyable()  = default;
    ~non_copyable() = default;

    non_copyable(non_copyable const&)                    = delete; // NOLINT(modernize-use-equals-delete)
    auto operator=(non_copyable const&) -> non_copyable& = delete; // NOLINT(modernize-use-equals-delete)
};

////////////////////////////////////////////////////////////

}
