// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/audio/Playlist.hpp"

#include "tcob/audio/Source.hpp"
#include "tcob/core/ServiceLocator.hpp"

namespace tcob::audio {

playlist::playlist()
{
    Volume.Changed.connect([&](auto const val) {
        for (auto* source : _playing) {
            source->Volume = val;
        }
    });
}

playlist::~playlist() = default;

void playlist::add(string const& name, source* source)
{
    _sources[name] = source;
}

void playlist::play(string const& name)
{
    auto* source {_sources.at(name)};
    if (!_deferred) {
        locate_service<task_manager>().run_deferred([&]() -> task_status { return update(); });
    }

    play(source);
}

void playlist::queue(string const& name)
{
    auto* source {_sources.at(name)};
    if (!_deferred) {
        locate_service<task_manager>().run_deferred([&]() -> task_status { return update(); });
    }

    _waiting.push(source);
}

auto playlist::update() -> task_status
{
    for (auto it {_playing.begin()}; it != _playing.end();) {
        if ((*it)->get_status() == playback_status::Stopped) {
            it = _playing.erase(it);
        } else {
            ++it;
        }
    }
    if (_playing.empty() && !_waiting.empty()) {
        play(_waiting.front());
        _waiting.pop();
    }

    _deferred = !_playing.empty();
    return _deferred ? task_status::Running : task_status::Finished;
}

void playlist::play(source* source)
{
    source->Volume = Volume;
    source->play();
    _playing.push_back(source);
}

}
