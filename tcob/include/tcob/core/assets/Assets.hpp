// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob::assets {
////////////////////////////////////////////////////////////

enum class asset_status : u8 {
    Unloaded,
    Created,
    Loading,
    Loaded,
    Error
};

struct stat {
    asset_status Status;
    i32          UseCount;
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
class asset;

template <typename T>
class asset_ptr;

template <typename T>
class bucket;

class group;

class library;

template <typename T>
class loader;

class loader_manager;

}
