// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob::gl {
ObjectRegistry Object::Registry = ObjectRegistry();

Object::Object()
{
    Registry.register_object(this);
}

Object::Object(Object&& other) noexcept
    : ID { std::exchange(other.ID, 0) }
{
    Registry.unregister_object(&other);
}

auto Object::operator=(Object&& other) noexcept -> Object&
{
    std::swap(ID, other.ID);
    Registry.unregister_object(&other);
    return *this;
}

void Object::destroy()
{
    Registry.unregister_object(this);

    if (ID) {
        do_destroy();
        ID = 0;
    }
}
}