// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/app/Platform.hpp"

#include <any>
#include <compare>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "../audio/audio_codecs/AudioCodecs.hpp"
#include "../data/config_parsers/ConfigParsers.hpp"
#include "../gfx/image_codecs/ImageCodecs.hpp"

#include "loaders/ConfigAssetLoader.hpp"

#include "tcob/app/Game.hpp"
#include "tcob/audio/AudioSystem.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Logger.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/assets/AssetGroup.hpp"
#include "tcob/core/assets/AssetLoader.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Magic.hpp"
#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigConversions.hpp"
#include "tcob/data/ConfigFile.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/RenderSystem.hpp"

#if defined(TCOB_ENABLE_RENDERER_OPENGL45)
    #include "../gfx/backend/gl45/GLRenderSystem.hpp"
#endif
#if defined(TCOB_ENABLE_RENDERER_OPENGLES30)
    #include "../gfx/backend/gles30/GLES30RenderSystem.hpp"
#endif
#if defined(TCOB_ENABLE_RENDERER_OPENGLES20)
    #include "../gfx/backend/gles20/GLES20RenderSystem.hpp"
#endif
#if defined(TCOB_ENABLE_RENDERER_NULL)
    #include "../gfx/backend/null/NullRenderSystem.hpp"
#endif

#if defined(_MSC_VER)
    #define WIN32_LEAN_AND_MEAN
    #include "Windows.h"
#else
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

namespace tcob {

platform::platform(bool headless, game::init const& ginit)
{
    //  file system
    if (!headless) {
        io::detail::init(ginit.Path.c_str(), ginit.Name, ginit.OrgName);
    } else {
        io::detail::simple_init(ginit.Path.c_str());
    }

    //  logger
    if (ginit.LogFile.empty()) {
        register_service<logger>(std::make_shared<null_logger>());
    } else if (ginit.LogFile == "stdout") {
        register_service<logger>(std::make_shared<stdout_logger>());
    } else {
        register_service<logger>(std::make_shared<file_logger>(ginit.LogFile));
    }
    logger::Info("starting");

    //  SDL
    InitSDL();

    // locales
    init_locales();

    // magic signatures
    InitSignatures();

    // task manager
    register_service<task_manager>(std::make_shared<task_manager>(
        ginit.WorkerThreads
            ? *ginit.WorkerThreads
            : static_cast<isize>(std::thread::hardware_concurrency())));

    // init config formats
    InitConfigFormats();

    // init image codecs
    InitImageCodecs();

    // init audio codecs
    InitAudioCodecs();

    // init truetype font engine
    InitFontEngines();

    // init input
    auto input {register_service<input::system>()};

    // init assets
    auto factory {register_service<assets::loader_manager::factory>()};
    factory->add({".ini", ".json", ".xml", ".yaml"},
                 [](assets::group& group) {
                     return std::make_unique<detail::cfg_asset_loader_manager>(group);
                 });

    if (!headless) {
        // init audio system
        register_service<audio::system>();                                   // DON'T MOVE AGAIN

        _configFile = std::make_unique<data::config_file>(ginit.ConfigFile); // load config
        _configFile->merge(*ginit.ConfigDefaults, false);                    // merge config with default

        // init render system
        init_render_system(ginit.Name);
    } else {
#if defined(TCOB_ENABLE_RENDERER_NULL)
        register_service<gfx::render_system, gfx::null::null_render_system>();
#endif
    }
}

platform::~platform()
{
    _configFile = nullptr;
    remove_services();

    //  file system
    logger::Info("exiting");
    remove_service<logger>();
    io::detail::done();

    //  SDL
    SDL_Quit();

    //  FreeType
    gfx::font::Done();
}

void platform::remove_services() const
{
    remove_service<input::system>();
    remove_service<audio::system>();
    remove_service<gfx::render_system>();
    remove_service<task_manager>();

    remove_service<assets::loader_manager::factory>();
    remove_service<data::text_reader::factory>();
    remove_service<data::text_writer::factory>();
    remove_service<data::binary_reader::factory>();
    remove_service<data::binary_writer::factory>();
    remove_service<gfx::image_decoder::factory>();
    remove_service<gfx::image_encoder::factory>();
    remove_service<gfx::animated_image_decoder::factory>();
    remove_service<gfx::animated_image_encoder::factory>();
    remove_service<audio::decoder::factory>();
    remove_service<audio::encoder::factory>();
    remove_service<gfx::render_system::factory>();
}

auto platform::HeadlessInit(char const* argv0, path logFile) -> platform
{
    return {true,
            {.Path           = argv0,
             .Name           = "",
             .OrgName        = "",
             .LogFile        = std::move(logFile),
             .ConfigFile     = "config.ini",
             .ConfigDefaults = {},
             .WorkerThreads  = std::nullopt}};
}

auto platform::IsRunningOnWine() -> bool
{
#if defined(_MSC_VER)
    auto* ntdll {GetModuleHandleA("ntdll.dll")};
    return ntdll ? GetProcAddress(ntdll, "wine_get_version") != nullptr : false;
#else
    return false;
#endif
}

auto platform::process_events() const -> bool
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
        case SDL_EVENT_JOYSTICK_AXIS_MOTION:
        case SDL_EVENT_JOYSTICK_HAT_MOTION:
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
        case SDL_EVENT_JOYSTICK_BUTTON_UP:
        case SDL_EVENT_JOYSTICK_ADDED:
        case SDL_EVENT_JOYSTICK_REMOVED:
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
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

auto platform::preferred_locales() const -> std::vector<locale> const&
{
    return _locales;
}

auto display_mode::operator<=>(display_mode const& other) const -> std::partial_ordering
{
    if (auto const cmp {Size.Width <=> other.Size.Width}; cmp != 0) { return cmp; }
    if (auto const cmp {Size.Height <=> other.Size.Height}; cmp != 0) { return cmp; }
    if (auto const cmp {RefreshRate <=> other.RefreshRate}; cmp != 0) { return cmp; }
    return other.PixelDensity <=> PixelDensity;
}

auto platform::displays() const -> std::map<i32, display>
{
    std::map<i32, display> retValue;

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

auto platform::get_desktop_mode(i32 display) const -> display_mode
{
    auto const* mode {SDL_GetDesktopDisplayMode(display)};
    return {.Size         = {mode->w, mode->h},
            .PixelDensity = mode->pixel_density,
            .RefreshRate  = mode->refresh_rate};
}

auto platform::was_paused() const -> bool
{
    return _wasPaused;
}

auto platform::config() const -> data::config_file&
{
    return *_configFile;
}

void platform::init_locales()
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

void platform::init_render_system(string const& windowTitle)
{
    auto rsFactory {register_service<gfx::render_system::factory>()};
#if defined(TCOB_ENABLE_RENDERER_OPENGL45)
    rsFactory->add({"OPENGL45"}, std::make_shared<gfx::gl45::gl_render_system>);
#endif
#if defined(TCOB_ENABLE_RENDERER_OPENGLES30)
    rsFactory->add({"OPENGLES30"}, std::make_shared<gfx::gles30::gl_render_system>);
#endif
#if defined(TCOB_ENABLE_RENDERER_OPENGLES20)
    rsFactory->add({"OPENGLES20"}, std::make_shared<gfx::gles20::gl_render_system>);
#endif
#if defined(TCOB_ENABLE_RENDERER_NULL)
    rsFactory->add({"NULL"}, std::make_shared<gfx::null::null_render_system>);
#endif

    auto video {(*_configFile)[Cfg::Video::Name].as<gfx::video_config>()};

    string renderer {video.RenderSystem};

    // create rendersystem (and window (and context))
    logger::Info("RenderSystem: {}", renderer);

    auto renderSystem {rsFactory->create(renderer)};
    if (!renderSystem) { throw std::runtime_error("Render system creation failed!"); }

    register_service<gfx::render_system>(renderSystem);
    auto& window {renderSystem->init_window(video, windowTitle, displays().begin()->second.DesktopMode.Size)};
    window.FullScreen.Changed.connect([this](bool value) {
        (*_configFile)[Cfg::Video::Name][Cfg::Video::fullscreen] = value;
    });
    window.VSync.Changed.connect([this](bool value) {
        (*_configFile)[Cfg::Video::Name][Cfg::Video::vsync] = value;
    });
    window.Resized.connect([this, &window](auto const&) {
        (*_configFile)[Cfg::Video::Name][Cfg::Video::use_desktop_resolution] = (window.Size == displays().begin()->second.DesktopMode.Size);
        (*_configFile)[Cfg::Video::Name][Cfg::Video::resolution]             = window.Size();
    });

    FrameLimit.Changed.connect([this](i32 value) {
        (*_configFile)[Cfg::Video::Name][Cfg::Video::frame_limit] = value;
    });
    FrameLimit = (*_configFile)[Cfg::Video::Name][Cfg::Video::frame_limit].as<i32>();

    logger::Info("Device: {}", renderSystem->device_name());

#if defined(_MSC_VER)
    SDL_SetWindowsMessageHook([](void* userdata, tagMSG* msg) -> bool {
        auto* plt {reinterpret_cast<platform*>(userdata)};
        plt->_wasPaused = msg->message == WM_NCLBUTTONDOWN; // left click on title bar
        return true;
    },
                              this);
#endif
}

void platform::InitSDL()
{
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS);

    i32 const version {SDL_GetVersion()};
    logger::Info("SDL version: {}.{}.{}", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
}

void platform::InitSignatures()
{
    using namespace io;

    // image
    magic::add_signature({.Extension = ".bmp", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'B', 'M'}}}});
    magic::add_signature({.Extension = ".bsi", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'B', 'S', 'I'}}}});
    magic::add_signature({.Extension = ".gif", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'G', 'I', 'F', '8', '7', 'a'}}}});
    magic::add_signature({.Extension = ".gif", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'G', 'I', 'F', '8', '9', 'a'}}}});
    magic::add_signature({.Extension = ".pcx", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {0x0a, 0x05, 0x00}}}});
    magic::add_signature({.Extension = ".pcx", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {0x0a, 0x05, 0x01}}}});
    magic::add_signature({.Extension = ".png", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a}}}});
    magic::add_signature({.Extension = ".pnm", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'P', '1'}}}});
    magic::add_signature({.Extension = ".pnm", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'P', '2'}}}});
    magic::add_signature({.Extension = ".pnm", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'P', '3'}}}});
    magic::add_signature({.Extension = ".qoi", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'q', 'o', 'i', 'f'}}}});
    magic::add_signature({.Extension = ".webp", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'R', 'I', 'F', 'F'}}, {.Offset = 8, .Bytes = {'W', 'E', 'B', 'P'}}}});
    magic::add_signature({.Extension = ".ogv", .Group = "image", .Parts = {{.Offset = 0, .Bytes = {'O', 'g', 'g', 'S'}}, {.Offset = 29, .Bytes = {'t', 'h', 'e', 'o', 'r', 'a'}}}});
    magic::add_signature({.Extension = ".tga", .Group = "image", .Parts = {{.Offset = -18, .Bytes = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'}}}});
    // audio
    magic::add_signature({.Extension = ".bsa", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {'B', 'S', 'A'}}}});
    magic::add_signature({.Extension = ".flac", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {'f', 'L', 'a', 'C'}}}});
    magic::add_signature({.Extension = ".mp3", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {0xff, 0xfb}}}});
    magic::add_signature({.Extension = ".mp3", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {0xff, 0xf3}}}});
    magic::add_signature({.Extension = ".mp3", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {0xff, 0xf2}}}});
    magic::add_signature({.Extension = ".mp3", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {'I', 'D', '3'}}}});
    magic::add_signature({.Extension = ".wav", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {'R', 'I', 'F', 'F'}}, {.Offset = 8, .Bytes = {'W', 'A', 'V', 'E'}}}});
    magic::add_signature({.Extension = ".ogg", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {'O', 'g', 'g', 'S'}}, {.Offset = 29, .Bytes = {'v', 'o', 'r', 'b', 'i', 's'}}}});
    magic::add_signature({.Extension = ".opus", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {'O', 'g', 'g', 'S'}}, {.Offset = 28, .Bytes = {'O', 'p', 'u', 's', 'H', 'e', 'a', 'd'}}}});
    magic::add_signature({.Extension = ".mid", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {'M', 'T', 'h', 'd'}}}});
    magic::add_signature({.Extension = ".it", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {'I', 'M', 'P', 'M'}}}});
    magic::add_signature({.Extension = ".xm", .Group = "audio", .Parts = {{.Offset = 0, .Bytes = {0x45, 0x78, 0x74, 0x65, 0x6e, 0x64, 0x65, 0x64, 0x20, 0x4d, 0x6f, 0x64, 0x75, 0x6c, 0x65}}}});
    magic::add_signature({.Extension = ".s3m", .Group = "audio", .Parts = {{.Offset = 44, .Bytes = {'S', 'C', 'R', 'M'}}}});
    // config
    magic::add_signature({.Extension = ".bsbd", .Group = "config", .Parts = {{.Offset = 0, .Bytes = {'B', 'S', 'B', 'D', 1}}}});
}

template <typename T>
static auto make_unique() -> std::unique_ptr<T>
{
    return std::make_unique<T>();
}

void platform::InitConfigFormats()
{
    // text
    // readers
    auto trFactory {register_service<data::text_reader::factory>()};
    trFactory->add({".ini"}, &make_unique<data::detail::ini_reader>);
    trFactory->add({".json"}, &make_unique<data::detail::json_reader>);
    trFactory->add({".xml"}, &make_unique<data::detail::xml_reader>);
    trFactory->add({".yaml"}, &make_unique<data::detail::yaml_reader>);

    // writers
    auto twFactory {register_service<data::text_writer::factory>()};
    twFactory->add({".ini"}, &make_unique<data::detail::ini_writer>);
    twFactory->add({".json"}, &make_unique<data::detail::json_writer>);
    twFactory->add({".xml"}, &make_unique<data::detail::xml_writer>);
    twFactory->add({".yaml"}, &make_unique<data::detail::yaml_writer>);

    // binary
    // readers
    auto brFactory {register_service<data::binary_reader::factory>()};
    brFactory->add({".bsbd"}, &make_unique<data::detail::bsbd_reader>);

    // writers
    auto bwFactory {register_service<data::binary_writer::factory>()};
    bwFactory->add({".bsbd"}, &make_unique<data::detail::bsbd_writer>);
}

void platform::InitImageCodecs()
{
    // decoders
    auto idFactory {register_service<gfx::image_decoder::factory>()};
    idFactory->add({".bmp"}, &make_unique<gfx::detail::bmp_decoder>);
    idFactory->add({".bsi"}, &make_unique<gfx::detail::bsi_decoder>);
    idFactory->add({".tga"}, &make_unique<gfx::detail::tga_decoder>);
    idFactory->add({".pcx"}, &make_unique<gfx::detail::pcx_decoder>);
    idFactory->add({".pnm"}, &make_unique<gfx::detail::pnm_decoder>);
    idFactory->add({".gif"}, &make_unique<gfx::detail::gif_decoder>);
    idFactory->add({".png"}, &make_unique<gfx::detail::png_decoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)
    idFactory->add({".qoi"}, &make_unique<gfx::detail::qoi_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    idFactory->add({".webp"}, &make_unique<gfx::detail::webp_decoder>);
#endif

    // encoders
    auto ieFactory {register_service<gfx::image_encoder::factory>()};
    ieFactory->add({".bmp"}, &make_unique<gfx::detail::bmp_encoder>);
    ieFactory->add({".bsi"}, &make_unique<gfx::detail::bsi_encoder>);
    ieFactory->add({".tga"}, &make_unique<gfx::detail::tga_encoder>);
    ieFactory->add({".pcx"}, &make_unique<gfx::detail::pcx_encoder>);
    ieFactory->add({".png"}, &make_unique<gfx::detail::png_encoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)
    ieFactory->add({".qoi"}, &make_unique<gfx::detail::qoi_encoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    ieFactory->add({".webp"}, &make_unique<gfx::detail::webp_encoder>);
#endif

    // animated
    // decoders
    auto iadFactory {register_service<gfx::animated_image_decoder::factory>()};
    iadFactory->add({".gif"}, &make_unique<gfx::detail::gif_decoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    iadFactory->add({".webp"}, &make_unique<gfx::detail::webp_anim_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_GFX_THEORA)
    iadFactory->add({".ogv"}, &make_unique<gfx::detail::theora_decoder>);
#endif

    // encoders
    auto iaeFactory {register_service<gfx::animated_image_encoder::factory>()};
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    iaeFactory->add({".webp"}, &make_unique<gfx::detail::webp_anim_encoder>);
#endif
}

void platform::InitAudioCodecs()
{
    // decoders
    auto adFactory {register_service<audio::decoder::factory>()};
    adFactory->add({".bsa"}, &make_unique<audio::detail::bsa_decoder>);
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)
    adFactory->add({".flac"}, &make_unique<audio::detail::flac_decoder>);
    adFactory->add({".mp3"}, &make_unique<audio::detail::mp3_decoder>);
    adFactory->add({".wav"}, &make_unique<audio::detail::wav_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_VORBIS)
    adFactory->add({".ogg"}, &make_unique<audio::detail::vorbis_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_OPUS)
    adFactory->add({".opus"}, &make_unique<audio::detail::opus_decoder>);
#endif
#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)
    adFactory->add({".mid"}, &make_unique<audio::detail::midi_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_LIBXMP)
    adFactory->add({".it", ".mod", ".s3m", ".xm"}, &make_unique<audio::detail::xmp_decoder>);
#endif

    // encoders
    auto aeFactory {register_service<audio::encoder::factory>()};
    aeFactory->add({".bsa"}, &make_unique<audio::detail::bsa_encoder>);
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)
    aeFactory->add({".wav"}, &make_unique<audio::detail::wav_encoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_VORBIS)
    aeFactory->add({".ogg"}, &make_unique<audio::detail::vorbis_encoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_OPUS)
    aeFactory->add({".opus"}, &make_unique<audio::detail::opus_encoder>);
#endif
}

void platform::InitFontEngines()
{
    /// init ttf engines
    gfx::font::Init();
}

////////////////////////////////////////////////////////////
single_instance::single_instance(string const& name)
{
#if defined(_MSC_VER)
    HANDLE mutex {CreateMutex(nullptr, TRUE, ("Global\\" + name).c_str())};
    if (mutex == nullptr) { return; }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);
        return;
    }

    _handle = mutex;
    _locked = true;
#else
    int fd {open(("/tmp/" + name + ".lock").c_str(), O_CREAT | O_RDWR, 0666)};
    if (fd == -1) { return; }

    if (lockf(fd, F_TLOCK, 0) == -1) {
        close(fd);
        return;
    }

    _handle = fd;
    _locked = true;
#endif
}

single_instance::~single_instance()
{
#if defined(_MSC_VER)
    if (_locked) {
        CloseHandle(std::any_cast<HANDLE>(_handle));
    }
#else
    if (_locked) {
        close(std::any_cast<int>(_handle));
    }
#endif
}

single_instance::operator bool() const
{
    return _locked;
}

}
