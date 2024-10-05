// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLESObject.hpp"

namespace tcob::gfx::gles30 {

void gl_object_registry::destroy_all_objects()
{
    while (!_objects.empty()) {
        _objects[0]->destroy();
    }
}

void gl_object_registry::register_object(gl_object* res)
{
    _objects.push_back(res);
}

void gl_object_registry::unregister_object(gl_object const* res) noexcept
{
    std::erase(_objects, res);
}

////////////////////////////////////////////////////////////

auto static GetRegistry() -> gl_object_registry&
{
    static gl_object_registry registry;
    return registry;
}

gl_object::gl_object()
{
    GetRegistry().register_object(this);
}

gl_object::gl_object(gl_object&& other) noexcept
    : ID {std::exchange(other.ID, 0)}
{
    GetRegistry().unregister_object(&other);
}

auto gl_object::operator=(gl_object&& other) noexcept -> gl_object&
{
    std::swap(ID, other.ID);
    GetRegistry().unregister_object(&other);
    return *this;
}

void gl_object::DestroyAll()
{
    GetRegistry().destroy_all_objects();
}

void gl_object::destroy()
{
    GetRegistry().unregister_object(this);

    if (ID) {
        do_destroy();
        ID = 0;
    }
}
}
