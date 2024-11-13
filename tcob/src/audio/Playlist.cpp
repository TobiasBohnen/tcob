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

playlist::~playlist()
{
    for (auto* source : _playing) { source->stop(); }
    locate_service<task_manager>().cancel_deferred(_deferred);
}

void playlist::add(string const& name, source* source)
{
    _sources[name] = source;
}

void playlist::play(string const& name)
{
    auto* source {_sources.at(name)};
    launch_task();
    play(source);
}

void playlist::queue(string const& name)
{
    auto* source {_sources.at(name)};
    launch_task();
    _waiting.push(source);
}

void playlist::clear_queue()
{
    _waiting = {};
}

void playlist::launch_task()
{
    if (_deferred == INVALID_ID) {
        _deferred = locate_service<task_manager>().run_deferred([&](def_task& ctx) { update(ctx); });
    }
}

auto playlist::update(def_task& ctx) -> void
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

    ctx.Finished = _playing.empty();
    if (ctx.Finished) { _deferred = 0; }
}

void playlist::play(source* source)
{
    source->Volume = Volume();
    source->play();
    _playing.push_back(source);
}

}
