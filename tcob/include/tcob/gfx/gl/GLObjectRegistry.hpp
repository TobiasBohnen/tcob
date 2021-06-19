// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <vector>

namespace tcob::gl {
class ObjectRegistry final {
public:
    ObjectRegistry() = default;
    ~ObjectRegistry() = default;
    ObjectRegistry(const ObjectRegistry&) = delete;
    auto operator=(const ObjectRegistry& other) -> ObjectRegistry& = delete;

    void register_object(Object* res);
    void unregister_object(const Object* res) noexcept;
    void destroy_all_objects();

private:
    std::vector<Object*> _resources;
};
}