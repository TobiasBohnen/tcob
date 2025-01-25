// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Interfaces.hpp"

namespace tcob::gfx::gles30 {
////////////////////////////////////////////////////////////

class gl_object;

class gl_object_registry final : public non_copyable { // TODO: move to GLContext
public:
    void register_object(gl_object* res);
    void unregister_object(gl_object const* res) noexcept;
    void destroy_all_objects();

private:
    std::vector<gl_object*> _objects;
};

////////////////////////////////////////////////////////////

class gl_object : public non_copyable {
    friend void gl_object_registry::destroy_all_objects();

public:
    gl_object();
    gl_object(gl_object&& other) noexcept;
    auto operator=(gl_object&& other) noexcept -> gl_object&;
    virtual ~gl_object() = default;

    u32 ID {0};

    void static DestroyAll();

protected:
    void destroy();
    void virtual do_destroy() = 0;
};
}
