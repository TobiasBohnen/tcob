// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

namespace tcob::gl {
class Object;

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

////////////////////////////////////////////////////////////

class Object {
public:
    Object();
    virtual ~Object() = default;

    Object(const Object&) = delete;
    auto operator=(const Object&) -> Object& = delete;

    Object(Object&& other) noexcept;
    auto operator=(Object&& other) noexcept -> Object&;

    u32 ID { 0 };

    static void DestroyAll()
    {
        Registry.destroy_all_objects();
    }

protected:
    friend void ObjectRegistry::destroy_all_objects();

    void destroy();
    virtual void do_destroy() = 0;

private:
    static ObjectRegistry Registry;
};
}