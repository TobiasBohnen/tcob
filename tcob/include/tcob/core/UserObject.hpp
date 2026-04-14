// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <typeindex>
#include <utility>

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API user_object final {
public:
    user_object() = default;

    template <typename T>
    auto get() const -> T* { return _type == typeid(T) ? static_cast<T*>(_data.get()) : nullptr; }

    auto has_value() const -> bool;
    auto type() const -> std::type_index;
    void reset();

    template <typename T>
    static auto Create(std::shared_ptr<T> ptr) -> user_object { return {std::move(ptr), typeid(T)}; }

    template <typename T>
    static auto Make(auto&&... args) -> user_object { return {std::make_shared<T>(args...), typeid(T)}; }

private:
    user_object(std::shared_ptr<void> ptr, std::type_index type);

    std::shared_ptr<void> _data;
    std::type_index       _type {typeid(void)};
};

}
