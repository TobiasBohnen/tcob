// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/scripting/Scripting.hpp"
#include "tcob/scripting/Wrapper.hpp"

namespace tcob::scripting {
////////////////////////////////////////////////////////////

template <typename ScriptImpl>
class script : public non_copyable {
public:
    template <typename R = void>
    auto run_file(path const& file) const -> result<R>;
    template <typename R = void>
    auto run(string_view script, string const& name = "main") const -> result<R>;

    template <typename T>
    auto create_wrapper(string const& name);

protected:
    script()  = default;
    ~script() = default;

    void clear_wrappers();

private:
    auto get_impl() -> ScriptImpl*;
    auto get_impl() const -> ScriptImpl const*;

    std::vector<std::shared_ptr<detail::wrapper_base>> _wrappers;
};

}

#include "Script.inl"
