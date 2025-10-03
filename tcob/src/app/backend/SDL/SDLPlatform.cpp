// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "SDLPlatform.hpp"

#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include <SDL3/SDL.h>

#include "tcob/app/Game.hpp"
#include "tcob/app/Platform.hpp"
#include "tcob/audio/Audio.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Logger.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigConversions.hpp"
#include "tcob/data/ConfigFile.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderSystem.hpp"

#if defined(TCOB_ENABLE_RENDERER_OPENGL45)
    #include "gfx/gl45/GLRenderSystem.hpp"
#endif
#if defined(TCOB_ENABLE_RENDERER_OPENGLES30)
    #include "gfx/gles30/GLES30RenderSystem.hpp"
#endif

#include "audio/SDLAudioSystem.hpp"
#include "input/SDLInputSystem.hpp"

#include "../null/audio/NullAudioSystem.hpp"
#include "../null/gfx/NullRenderSystem.hpp"
#include "../null/input/NullInputSystem.hpp"

#if defined(_MSC_VER)
    #define WIN32_LEAN_AND_MEAN
    #include "Windows.h"
#endif

namespace tcob {

template <typename T>
static auto make_shared() -> std::shared_ptr<T>
{
    return std::make_shared<T>();
}

////////////////////////////////////////////////////////////

sdl_platform::sdl_platform(bool headless, game::init const& ginit)
    : platform {headless, ginit}
{
    InitSDL();

    init_input_system();
    init_locales();

    if (!headless) {
        init_audio_system(); // <-- DON'T MOVE OUTSIDE HEADLESS
        init_render_system(ginit.Name);
        process_events();    // gamepad add events
    } else {
        register_service<gfx::render_system, gfx::null::null_render_system>();
    }
}

sdl_platform::~sdl_platform()
{
    remove_service<input::system>();
    remove_service<input::system::factory>();

    remove_service<audio::system>();
    remove_service<audio::system::factory>();

    remove_service<gfx::render_system>();
    remove_service<gfx::render_system::factory>();

    SDL_Quit();
}

auto sdl_platform::process_events() const -> bool
{
    SDL_Event ev;
    auto&     inputMgr {locate_service<input::system>()};
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_EVENT_DROP_FILE: {
            DropFile(ev.drop.data);
        } break;
        case SDL_EVENT_QUIT: return false;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_TEXT_INPUT:
        case SDL_EVENT_TEXT_EDITING:
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL:
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED:
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
        case SDL_EVENT_CLIPBOARD_UPDATE:
            inputMgr.process_events(&ev);
            break;
        default:
            if (ev.type >= SDL_EVENT_WINDOW_FIRST && ev.type <= SDL_EVENT_WINDOW_LAST) {
                locate_service<gfx::render_system>().window().process_events(&ev);
            }
            break;
        }
    }

    return true;
}

auto sdl_platform::preferred_locales() const -> std::vector<locale> const&
{
    return _locales;
}

auto sdl_platform::displays() const -> std::map<i32, gfx::display>
{
    std::map<i32, gfx::display> retValue;

    i32   numDisplays {};
    auto* displayID {SDL_GetDisplays(&numDisplays)};

    for (i32 i {0}; i < numDisplays; ++i) {
        i32    numModes {};
        auto** displayModes {SDL_GetFullscreenDisplayModes(displayID[i], &numModes)};
        for (i32 j {0}; j < numModes; ++j) {
            auto* mode {displayModes[j]};
            retValue[mode->displayID].Modes.insert({.Size         = {mode->w, mode->h},
                                                    .PixelDensity = mode->pixel_density,
                                                    .RefreshRate  = mode->refresh_rate});
        }

        auto const* dmode {SDL_GetDesktopDisplayMode(displayID[i])};
        retValue[dmode->displayID].DesktopMode = {.Size         = {dmode->w, dmode->h},
                                                  .PixelDensity = dmode->pixel_density,
                                                  .RefreshRate  = dmode->refresh_rate};
    }

    return retValue;
}

auto sdl_platform::get_desktop_mode(i32 display) const -> gfx::display_mode
{
    auto const* mode {SDL_GetDesktopDisplayMode(display)};
    return {.Size         = {mode->w, mode->h},
            .PixelDensity = mode->pixel_density,
            .RefreshRate  = mode->refresh_rate};
}

auto sdl_platform::window_frozen() const -> bool
{
    return _wasPaused;
}

void sdl_platform::init_locales()
{
    i32 count {};
    if (auto** sdlLocales {SDL_GetPreferredLocales(&count)}) {
        for (i32 i {0}; i < count; ++i) {
            locale loc {};
            auto*  sdlLocale {sdlLocales[i]};
            if (sdlLocale->language) {
                loc.Language = sdlLocale->language;
                if (sdlLocale->country) {
                    loc.Country = sdlLocale->country;
                }
            } else {
                break;
            }

            _locales.push_back(loc);
        }
        SDL_free(sdlLocales);
    }
}

void sdl_platform::init_audio_system()
{
    auto& factory {register_service<audio::system::factory>()};
    factory.add("SDL", &make_shared<audio::sdl_audio_system>);
    factory.add("NULL", &make_shared<audio::null::null_audio_system>);

    string audio {"SDL"};

    logger::Info("AudioSystem: {}", audio);

    auto system {factory.create(audio)};
    if (!system) { throw std::runtime_error("Audio system creation failed"); }

    register_service<audio::system>(system);
}

void sdl_platform::init_render_system(string const& windowTitle)
{
    auto& rsFactory {register_service<gfx::render_system::factory>()};
#if defined(TCOB_ENABLE_RENDERER_OPENGL45)
    rsFactory.add("OPENGL45", &make_shared<gfx::gl45::gl_render_system>);
#endif
#if defined(TCOB_ENABLE_RENDERER_OPENGLES30)
    rsFactory.add("OPENGLES30", &make_shared<gfx::gles30::gl_render_system>);
#endif
    rsFactory.add("NULL", &make_shared<gfx::null::null_render_system>);

    gfx::video_config video;
    if (!config().try_get(video, Cfg::Video::Name)) { throw std::runtime_error("Invalid video config"); }

    string renderer {video.RenderSystem};

    // create rendersystem (and window (and context))
    logger::Info("RenderSystem: {}", renderer);

    auto renderSystem {rsFactory.create(renderer)};
    if (!renderSystem) { throw std::runtime_error("Render system creation failed"); }

    register_service<gfx::render_system>(renderSystem);
    auto& window {renderSystem->init_window(video, windowTitle, displays().begin()->second.DesktopMode.Size)};
    window.FullScreen.Changed.connect([this](bool value) {
        config()[Cfg::Video::Name][Cfg::Video::fullscreen] = value;
    });
    window.VSync.Changed.connect([this](bool value) {
        config()[Cfg::Video::Name][Cfg::Video::vsync] = value;
    });
    window.Resized.connect([this, &window](auto const&) {
        config()[Cfg::Video::Name][Cfg::Video::use_desktop_resolution] = (window.Size == displays().begin()->second.DesktopMode.Size);
        config()[Cfg::Video::Name][Cfg::Video::resolution]             = *window.Size;
    });

    FrameLimit.Changed.connect([this](i32 value) {
        config()[Cfg::Video::Name][Cfg::Video::frame_limit] = value;
    });
    FrameLimit = config()[Cfg::Video::Name][Cfg::Video::frame_limit].as<i32>();

    logger::Info("Device: {}", renderSystem->device_name());

#if defined(_MSC_VER)
    SDL_SetWindowsMessageHook([](void* userdata, tagMSG* msg) -> bool {
        auto* plt {reinterpret_cast<sdl_platform*>(userdata)};
        plt->_wasPaused = msg->message == WM_NCLBUTTONDOWN; // left click on title bar
        return true;
    },
                              this);
#endif
}

void sdl_platform::init_input_system()
{
    auto& factory {register_service<input::system::factory>()};
    factory.add("SDL", &make_shared<input::sdl_input_system>);
    factory.add("NULL", &make_shared<input::null::null_input_system>);

    string input {"SDL"};

    logger::Info("InputSystem: {}", input);

    auto system {factory.create(input)};
    if (!system) { throw std::runtime_error("Input system creation failed"); }

    register_service<input::system>(system);
}

void sdl_platform::InitSDL()
{
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS);

    i32 const version {SDL_GetVersion()};
    logger::Info("SDL version: {}.{}.{}", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
}

}
