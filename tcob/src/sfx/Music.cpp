// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/sfx/Music.hpp>

#include "AudioIO.hpp"
#include <tcob/core/io/FileStream.hpp>

namespace tcob {

Music::Music()
    : _source { std::make_unique<al::Source>() }
//  , _buffer { std::make_shared<al::Buffer>() }
{
}

Music::~Music()
{
    stop();
    //_source->buffer(0);
    //_buffer = nullptr;
    _source = nullptr;
}

auto Music::open(const std::string& filename) -> bool
{

    return true;
}

void Music::play()
{
}

void Music::stop()
{
}

}