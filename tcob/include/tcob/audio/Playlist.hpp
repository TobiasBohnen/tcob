// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <queue>
#include <unordered_map>
#include <vector>

#include "tcob/audio/Audio.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/TaskManager.hpp"

namespace tcob::audio {
////////////////////////////////////////////////////////////

class TCOB_API playlist final : public non_copyable {
public:
    playlist();
    ~playlist();

    prop<f32> Volume {1.0f};

    void add(string const& name, source* source);

    void play(string const& name);

    void queue(string const& name);
    void clear_queue();

private:
    void launch_task();
    void update(def_task& ctx);

    void play(source* source);

    std::unordered_map<string, source*> _sources;

    std::vector<source*> _playing;
    std::queue<source*>  _waiting;

    uid _deferred {INVALID_ID};
};

}
