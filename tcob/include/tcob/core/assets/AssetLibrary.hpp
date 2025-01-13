// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/assets/Assets.hpp"

namespace tcob::assets {
////////////////////////////////////////////////////////////

class TCOB_API library final : public non_copyable {
public:
    explicit library();
    ~library();

    auto loading_progress() const -> f32;
    auto is_loading_complete() const -> bool;

    auto create_or_get_group(string const& groupName) -> group&;
    auto get_group(string const& groupName) const -> group*;
    auto has_group(string const& groupName) const -> bool;

    void load_group(string const& groupName);
    void load_all_groups();

    void unload_group(string const& groupName);
    void unload_all_groups();

    void destroy_group(string const& groupName);
    void destroy_all_groups();

    auto asset_stats(string const& groupName) const -> group_stats;

private:
    std::unordered_map<string, std::unique_ptr<group>> _groups {};
};

}
