// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Interfaces.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data {
////////////////////////////////////////////////////////////

class TCOB_API config_file final : public data::object, public non_copyable {
public:
    explicit config_file(string file);
    ~config_file() override;

    void save() const;

private:
    string _fileName;
};
}
