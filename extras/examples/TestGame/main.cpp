#include "StartScene.hpp"

#include <SDL.h>
#include <tcob/tcob.hpp>

int main(int argc, char* argv[])
{
    tcob::Game game { argc, argv, "TestGame" };
    game.start<StartScene>();
    return 0;
}