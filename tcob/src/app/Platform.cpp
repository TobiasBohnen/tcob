// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/app/Platform.hpp"

#include <thread>
#include <utility>

#include <SDL.h>

#include "../audio/audio_codecs/AudioCodecs.hpp"
#include "../data/config_parsers/ConfigParsers.hpp"
#include "../gfx/image_codecs/ImageCodecs.hpp"

#include "loaders/ConfigAssetLoader.hpp"
#include "loaders/RasterFontLoader.hpp"

#include "tcob/audio/AudioSystem.hpp"
#include "tcob/core/CommandQueue.hpp"
#include "tcob/core/Semaphore.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Magic.hpp"
#include "tcob/data/ConfigFile.hpp"
#include "tcob/data/ConfigTypes.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/RenderSystem.hpp"

#if defined(TCOB_ENABLE_RENDERER_OPENGL45)
    #include "../gfx/backend/gl45/GLRenderSystem.hpp"
#endif
#if defined(TCOB_ENABLE_RENDERER_OPENGLES30)
    #include "../gfx/backend/gles30/GLESRenderSystem.hpp"
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

platform::platform(game* game, game::init const& ginit)
    : _game {game}
{
    //  file system
    if (_game) {
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

    // global semaphore
    u32 const threads {ginit.AsyncLoadThreads != 0
                           ? ginit.AsyncLoadThreads
                           : std::thread::hardware_concurrency() * 2};
    register_service<semaphore>(std::make_shared<semaphore>(threads));

    // init config formats
    InitConfigFormats();

    // init command queue
    register_service<command_queue>();

    // init image codecs
    InitImageCodecs();

    // init audio codecs
    InitAudioCodecs();

    // init truetype font engine
    InitFontEngines();

    // init input
    auto input {register_service<input::system>()};
    input->KeyDown.connect<&platform::on_key_down>(this);

    // init assets
    auto factory {register_service<assets::loader_manager::factory>()};
    factory->add({".ini", ".json", ".xml", ".yaml"},
                 [](assets::group& group) { return std::make_unique<detail::cfg_asset_loader_manager>(group); });
    register_service<assets::library>();

    if (_game) {
        // init audio system
        register_service<audio::system>(); // DON'T MOVE AGAIN

        // load config
        auto config {std::make_shared<data::config_file>(ginit.ConfigFile)};
        register_service<data::config_file>(config);
        config->merge(_game->get_config_defaults(), false); // merge config with default

        // init render system
        init_render_system(*config);
    } else {
#if defined(TCOB_ENABLE_RENDERER_NULL)
        register_service<gfx::render_system, gfx::null::null_render_system>();
#endif
    }
}

platform::~platform()
{
    remove_services();
    _window = nullptr;

    //  file system
    logger::Info("exiting");
    remove_service<logger>();
    io::detail::done();

    //  SDL
    SDL_Quit();

    //  FreeType
    gfx::truetype_font_engine::Done();
}

void platform::remove_services() const
{
    remove_service<command_queue>();
    remove_service<assets::library>();
    remove_service<input::system>();
    remove_service<audio::system>();
    remove_service<gfx::render_system>();
    remove_service<data::config_file>();
    remove_service<semaphore>();

    remove_service<assets::loader_manager::factory>();
    remove_service<data::config::text_reader::factory>();
    remove_service<data::config::text_writer::factory>();
    remove_service<data::config::binary_reader::factory>();
    remove_service<data::config::binary_writer::factory>();
    remove_service<gfx::image_decoder::factory>();
    remove_service<gfx::image_encoder::factory>();
    remove_service<gfx::animated_image_decoder::factory>();
    remove_service<gfx::animated_image_encoder::factory>();
    remove_service<audio::decoder::factory>();
    remove_service<audio::encoder::factory>();
    remove_service<gfx::raster_font::loader::factory>();
    remove_service<gfx::render_system::factory>();
}

void platform::on_key_down(input::keyboard::event& ev)
{
    using namespace tcob::enum_ops;
    if (!ev.Repeat) {
        // Alt+Enter -> toggle fullscreen
        if (ev.ScanCode == input::scan_code::RETURN && (ev.KeyMods & input::key_mod::LeftAlt) == input::key_mod::LeftAlt) {
            _window->FullScreen = !_window->FullScreen();
        }
    }
}

auto platform::HeadlessInit(char const* argv0, path logFile) -> platform
{
    return {nullptr,
            {.Path             = argv0,
             .Name             = "",
             .ConfigFile       = "config.ini",
             .OrgName          = "",
             .LogFile          = std::move(logFile),
             .AsyncLoadThreads = 0}};
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

void platform::process_events() const
{
    SDL_Event ev;
    auto&     inputMgr {locate_service<input::system>()};
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_DROPFILE: {
            string path {ev.drop.file};
            SDL_free(ev.drop.file);
            _game->DropFile(path);
        } break;
        case SDL_QUIT:
            _game->queue_finish();
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_TEXTINPUT:
        case SDL_TEXTEDITING:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        case SDL_JOYAXISMOTION:
        case SDL_JOYHATMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
        case SDL_CONTROLLERAXISMOTION:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            inputMgr.process_events(&ev);
            break;
        case SDL_WINDOWEVENT:
            _window->process_events(&ev);
            break;
        default:
            break;
        }
    }
}

auto platform::get_preferred_locales() const -> std::vector<locale> const&
{
    return _locales;
}

auto platform::has_window() const -> bool
{
    return _window != nullptr;
}

auto platform::get_window() const -> gfx::window&
{
    return *_window;
}

auto platform::get_default_target() const -> gfx::default_render_target&
{
    return *_defaultTarget;
}

void platform::init_locales()
{
    if (auto* sdlLocales {SDL_GetPreferredLocales()}) {
        for (auto* sdlLocale {sdlLocales}; sdlLocale; ++sdlLocale) {
            locale loc {};
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

void platform::init_render_system(data::config_file const& config)
{
    auto rsFactory {register_service<gfx::render_system::factory>()};
#if defined(TCOB_ENABLE_RENDERER_OPENGL45)
    rsFactory->add({"OPENGL45"}, std::make_shared<gfx::gl45::gl_render_system>);
#endif
#if defined(TCOB_ENABLE_RENDERER_OPENGLES30)
    rsFactory->add({"OPENGLES30"}, std::make_shared<gfx::gles30::gl_render_system>);
#endif
#if defined(TCOB_ENABLE_RENDERER_NULL)
    rsFactory->add({"NULL"}, std::make_shared<gfx::null::null_render_system>);
#endif

    string renderer {config[Cfg::Video::Name][Cfg::Video::render_system].as<string>()};

    // create rendersystem
    logger::Info("RenderSystem: {}", renderer);
    auto renderSystem {rsFactory->create(renderer)};
    if (!renderSystem) { throw std::runtime_error("Render system creation failed!"); }
    register_service<gfx::render_system>(renderSystem);

    // get config
    auto const video {locate_service<data::config_file>()[Cfg::Video::Name].as<data::config::object>()};

    size_i const resolution {video[Cfg::Video::use_desktop_resolution].as<bool>()
                                 ? renderSystem->get_desktop_size(0)
                                 : video[Cfg::Video::resolution].as<size_i>()};

    // create window (and context)
    _window = std::unique_ptr<gfx::window> {new gfx::window(renderSystem->create_window(resolution))};

    _window->FullScreen.Changed.connect([](bool value) {
        locate_service<data::config_file>()[Cfg::Video::Name][Cfg::Video::fullscreen] = value;
    });
    _window->VSync.Changed.connect([](bool value) {
        locate_service<data::config_file>()[Cfg::Video::Name][Cfg::Video::vsync] = value;
    });
    _window->Size.Changed.connect([](size_i value) {
        auto& cfg {locate_service<data::config_file>()};
        cfg[Cfg::Video::Name][Cfg::Video::use_desktop_resolution] = value == locate_service<gfx::render_system>().get_desktop_size(0);
        cfg[Cfg::Video::Name][Cfg::Video::resolution]             = value;
    });

    _window->FullScreen(video[Cfg::Video::fullscreen].as<bool>());
    _window->VSync(video[Cfg::Video::vsync].as<bool>());
    _window->Size(resolution);

    _defaultTarget = std::make_unique<gfx::default_render_target>();

    _window->clear();
    _window->draw_to(*_defaultTarget);
    _window->swap_buffer();
}

void platform::InitSDL()
{
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);
    SDL_EventState(SDL_DROPTEXT, SDL_ENABLE);
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    SDL_version ver {};
    SDL_GetVersion(&ver);
    logger::Info("SDL version: {}.{}.{}", ver.major, ver.minor, ver.patch);
}

void platform::InitSignatures()
{
    using namespace io;

    // misc
    magic::add_signature({{{0, {'r', 'F', 'X', ' '}}}, ".rfx", "misc"});
    magic::add_signature({{{0, {'B', 'M', 'F'}}}, ".fnt", "misc"});
    // image
    magic::add_signature({{{0, {'B', 'M'}}}, ".bmp", "image"});
    magic::add_signature({{{0, {'B', 'S', 'I'}}}, ".bsi", "image"});
    magic::add_signature({{{0, {'G', 'I', 'F', '8', '7', 'a'}}}, ".gif", "image"});
    magic::add_signature({{{0, {'G', 'I', 'F', '8', '9', 'a'}}}, ".gif", "image"});
    magic::add_signature({{{0, {0x0a, 0x05, 0x00}}}, ".pcx", "image"});
    magic::add_signature({{{0, {0x0a, 0x05, 0x01}}}, ".pcx", "image"});
    magic::add_signature({{{0, {0x89, 0x50, 0x4e, 0x47, 0x0d, 0xa, 0x1a, 0x0a}}}, ".png", "image"});
    magic::add_signature({{{0, {'P', '1'}}}, ".pnm", "image"});
    magic::add_signature({{{0, {'P', '2'}}}, ".pnm", "image"});
    magic::add_signature({{{0, {'P', '3'}}}, ".pnm", "image"});
    magic::add_signature({{{0, {'q', 'o', 'i', 'f'}}}, ".qoi", "image"});
    magic::add_signature({{{0, {'R', 'I', 'F', 'F'}}, {8, {'W', 'E', 'B', 'P'}}}, ".webp", "image"});
    magic::add_signature({{{0, {'O', 'g', 'g', 'S'}}, {29, {'t', 'h', 'e', 'o', 'r', 'a'}}}, ".ogv", "image"});
    magic::add_signature({{{-18, {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'}}}, ".tga", "image"});
    // audio
    magic::add_signature({{{0, {'B', 'S', 'A'}}}, ".bsa", "audio"});
    magic::add_signature({{{0, {'f', 'L', 'a', 'C'}}}, ".flac", "audio"});
    magic::add_signature({{{0, {0xff, 0xfb}}}, ".mp3", "audio"});
    magic::add_signature({{{0, {0xff, 0xf3}}}, ".mp3", "audio"});
    magic::add_signature({{{0, {0xff, 0xf2}}}, ".mp3", "audio"});
    magic::add_signature({{{0, {'I', 'D', '3'}}}, ".mp3", "audio"});
    magic::add_signature({{{0, {'R', 'I', 'F', 'F'}}, {8, {'W', 'A', 'V', 'E'}}}, ".wav", "audio"});
    magic::add_signature({{{0, {'O', 'g', 'g', 'S'}}, {29, {'v', 'o', 'r', 'b', 'i', 's'}}}, ".ogg", "audio"});
    magic::add_signature({{{0, {'O', 'g', 'g', 'S'}}, {28, {'O', 'p', 'u', 's', 'H', 'e', 'a', 'd'}}}, ".opus", "audio"});
    magic::add_signature({{{0, {'M', 'T', 'h', 'd'}}}, ".mid", "audio"});
    magic::add_signature({{{0, {'I', 'M', 'P', 'M'}}}, ".it", "audio"});
    magic::add_signature({{{0, {0x45, 0x78, 0x74, 0x65, 0x6e, 0x64, 0x65, 0x64, 0x20, 0x4d, 0x6f, 0x64, 0x75, 0x6c, 0x65}}}, ".xm", "audio"});
    magic::add_signature({{{44, {'S', 'C', 'R', 'M'}}}, ".s3m", "audio"});
    // config
    magic::add_signature({{{0, {'B', 'S', 'B', 'D', 1}}}, ".bsbd", "config"});
}

void platform::InitConfigFormats()
{
    // text
    // readers
    auto trFactory {register_service<data::config::text_reader::factory>()};
    trFactory->add({".ini"}, std::make_unique<data::config::detail::ini_reader>);
    trFactory->add({".json"}, std::make_unique<data::config::detail::json_reader>);
    trFactory->add({".xml"}, std::make_unique<data::config::detail::xml_reader>);
    trFactory->add({".yaml"}, std::make_unique<data::config::detail::yaml_reader>);

    // writers
    auto twFactory {register_service<data::config::text_writer::factory>()};
    twFactory->add({".ini"}, std::make_unique<data::config::detail::ini_writer>);
    twFactory->add({".json"}, std::make_unique<data::config::detail::json_writer>);
    twFactory->add({".xml"}, std::make_unique<data::config::detail::xml_writer>);
    twFactory->add({".yaml"}, std::make_unique<data::config::detail::yaml_writer>);

    // binary
    // readers
    auto brFactory {register_service<data::config::binary_reader::factory>()};
    brFactory->add({".bsbd"}, std::make_unique<data::config::detail::bsbd_reader>);

    // writers
    auto bwFactory {register_service<data::config::binary_writer::factory>()};
    bwFactory->add({".bsbd"}, std::make_unique<data::config::detail::bsbd_writer>);
}

void platform::InitImageCodecs()
{
    // decoders
    auto idFactory {register_service<gfx::image_decoder::factory>()};
    idFactory->add({".bmp"}, std::make_unique<gfx::detail::bmp_decoder>);
    idFactory->add({".bsi"}, std::make_unique<gfx::detail::bsi_decoder>);
    idFactory->add({".tga"}, std::make_unique<gfx::detail::tga_decoder>);
    idFactory->add({".pcx"}, std::make_unique<gfx::detail::pcx_decoder>);
    idFactory->add({".pnm"}, std::make_unique<gfx::detail::pnm_decoder>);
    idFactory->add({".gif"}, std::make_unique<gfx::detail::gif_decoder>);
    idFactory->add({".png"}, std::make_unique<gfx::detail::png_decoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)
    idFactory->add({".qoi"}, std::make_unique<gfx::detail::qoi_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    idFactory->add({".webp"}, std::make_unique<gfx::detail::webp_decoder>);
#endif

    // encoders
    auto ieFactory {register_service<gfx::image_encoder::factory>()};
    ieFactory->add({".bmp"}, std::make_unique<gfx::detail::bmp_encoder>);
    ieFactory->add({".bsi"}, std::make_unique<gfx::detail::bsi_encoder>);
    ieFactory->add({".tga"}, std::make_unique<gfx::detail::tga_encoder>);
    ieFactory->add({".pcx"}, std::make_unique<gfx::detail::pcx_encoder>);
    ieFactory->add({".png"}, std::make_unique<gfx::detail::png_encoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)
    ieFactory->add({".qoi"}, std::make_unique<gfx::detail::qoi_encoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    ieFactory->add({".webp"}, std::make_unique<gfx::detail::webp_encoder>);
#endif

    // animated
    // decoders
    auto iadFactory {register_service<gfx::animated_image_decoder::factory>()};
    iadFactory->add({".gif"}, std::make_unique<gfx::detail::gif_anim_decoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    iadFactory->add({".webp"}, std::make_unique<gfx::detail::webp_anim_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_GFX_THEORA)
    iadFactory->add({".ogv"}, std::make_unique<gfx::detail::theora_decoder>);
#endif

    // encoders
    auto iaeFactory {register_service<gfx::animated_image_encoder::factory>()};
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    iaeFactory->add({".webp"}, std::make_unique<gfx::detail::webp_anim_encoder>);
#endif
}

void platform::InitAudioCodecs()
{
    // decoders
    auto adFactory {register_service<audio::decoder::factory>()};
    adFactory->add({".bsa"}, std::make_unique<audio::detail::bsa_decoder>);
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)
    adFactory->add({".flac"}, std::make_unique<audio::detail::flac_decoder>);
    adFactory->add({".mp3"}, std::make_unique<audio::detail::mp3_decoder>);
    adFactory->add({".wav"}, std::make_unique<audio::detail::wav_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_VORBIS)
    adFactory->add({".ogg"}, std::make_unique<audio::detail::vorbis_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_OPUS)
    adFactory->add({".opus"}, std::make_unique<audio::detail::opus_decoder>);
#endif
#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)
    adFactory->add({".mid"}, std::make_unique<audio::detail::midi_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_LIBXMP)
    adFactory->add({".it", ".mod", ".s3m", ".xm"}, std::make_unique<audio::detail::xmp_decoder>);
#endif

    // encoders
    auto aeFactory {register_service<audio::encoder::factory>()};
    aeFactory->add({".bsa"}, std::make_unique<audio::detail::bsa_encoder>);
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)
    aeFactory->add({".wav"}, std::make_unique<audio::detail::wav_encoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_VORBIS)
    aeFactory->add({".ogg"}, std::make_unique<audio::detail::vorbis_encoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_OPUS)
    aeFactory->add({".opus"}, std::make_unique<audio::detail::opus_encoder>);
#endif
}

void platform::InitFontEngines()
{
    /// init ttf engines
    gfx::truetype_font_engine::Init();

    /// init raster font loader
    auto rasFactory {register_service<gfx::raster_font::loader::factory>()};
    rasFactory->add({".ini", ".json", ".xml", ".yaml"}, std::make_unique<detail::ini_raster_font_loader>);
    rasFactory->add({".fnt"}, std::make_unique<detail::fnt_raster_font_loader>);
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
