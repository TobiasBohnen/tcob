// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>

namespace tcob {
enum class asset_status : u8 {
    Unloaded,
    Created,
    Loading,
    Loaded,
    Error
};
}

namespace tcob::assets {
////////////////////////////////////////////////////////////

struct stat {
    asset_status Status;
    isize        UseCount;
};

struct bucket_stats {
    std::unordered_map<string, stat>      Assets;
    std::unordered_map<asset_status, i32> Statuses;
};

struct group_stats {
    std::unordered_map<string, bucket_stats> Buckets;
};

////////////////////////////////////////////////////////////

template <typename T>
class bucket;

class group;

class library;

template <typename T>
class loader;

class loader_manager;

}
