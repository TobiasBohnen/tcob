// Copyright (c) 2025 Tobias Bohnen
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

    virtual void update(milliseconds deltaTime);

protected:
    updatable()                                                   = default;
    updatable(updatable const& other) noexcept                    = default;
    auto operator=(updatable const& other) noexcept -> updatable& = default;
    updatable(updatable&& other) noexcept                         = default;
    auto operator=(updatable&& other) noexcept -> updatable&      = default;

    virtual void on_update(milliseconds deltaTime);
};

////////////////////////////////////////////////////////////

class TCOB_API hybrid_updatable : public updatable {
public:
    virtual void fixed_update(milliseconds deltaTime);

protected:
    virtual void on_fixed_update(milliseconds deltaTime);
};

////////////////////////////////////////////////////////////

class TCOB_API non_copyable {
public:
    non_copyable(non_copyable const&)                    = delete;
    auto operator=(non_copyable const&) -> non_copyable& = delete;

protected:
    non_copyable()  = default;
    ~non_copyable() = default;
};

////////////////////////////////////////////////////////////

}
