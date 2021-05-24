// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLObjectRegistry.hpp>

#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob::gl {
void ObjectRegistry::destroy_all_objects()
{
    while (_resources.size() > 0) {
        _resources[0]->destroy();
    }

    _resources.clear();
}

void ObjectRegistry::register_object(Object* res)
{
    _resources.push_back(res);
}

void ObjectRegistry::unregister_object(const Object* res) noexcept
{
    if (_resources.empty()) {
        return;
    }

    _resources.erase(std::remove(_resources.begin(), _resources.end(), res), _resources.end());
}
}