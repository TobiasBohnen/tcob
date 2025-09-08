// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/app/Platform.hpp"

#include <any>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include "../audio/audio_codecs/AudioCodecs.hpp"
#include "../data/config_formats/ConfigFormats.hpp"
#include "../gfx/image_codecs/ImageCodecs.hpp"

#include "../gfx/FontEngine.hpp"

#include "loaders/ConfigAssetLoader.hpp"

#include "tcob/app/Game.hpp"
#include "tcob/audio/Audio.hpp"
#include "tcob/audio/Buffer.hpp"
#include "tcob/core/Logger.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/core/assets/AssetGroup.hpp"
#include "tcob/core/assets/AssetLoader.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Magic.hpp"
#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigConversions.hpp"
#include "tcob/data/ConfigFile.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Image.hpp"

#include "backend/SDL/SDLPlatform.hpp"

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

template <typename T>
static auto make_unique() -> std::unique_ptr<T>
{
    return std::make_unique<T>();
}

platform::platform(bool headless, game::init const& ginit)
{
    //  file system
    if (!headless) {
        io::detail::init(ginit.Name, ginit.OrgName);
    } else {
        io::detail::simple_init();
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

    InitSignatures();
    InitTaskManager(ginit.WorkerThreads);
    InitConfigFormats();
    InitImageCodecs();
    InitAudioCodecs();
    InitFontEngines();
    InitAssetFormats();

    if (!headless) {
        _configFile = std::make_unique<data::config_file>(ginit.ConfigFile); // load config
        _configFile->merge(*ginit.ConfigDefaults, false);                    // merge config with default
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

    //  FreeType
    gfx::truetype_font_engine::Done();
}

void platform::remove_services() const
{
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

auto platform::Init(game::init const& ginit) -> std::shared_ptr<platform>
{
    return std::shared_ptr<platform> {new sdl_platform {false, ginit}};
}

auto platform::HeadlessInit(path const& logFile) -> std::shared_ptr<platform>
{
    return std::shared_ptr<platform> {
        new sdl_platform {true,
                          {.Name           = "",
                           .OrgName        = "",
                           .LogFile        = logFile,
                           .ConfigFile     = "config.ini",
                           .ConfigDefaults = {},
                           .WorkerThreads  = std::nullopt}}};
}

auto platform::config() const -> data::config_file&
{
    return *_configFile;
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

void platform::InitConfigFormats()
{
    // text
    // readers
    auto trFactory {register_service<data::text_reader::factory>()};
    trFactory->add(".ini", &make_unique<data::detail::ini_reader>);
    trFactory->add(".json", &make_unique<data::detail::json_reader>);
    trFactory->add(".xml", &make_unique<data::detail::xml_reader>);
    trFactory->add(".yaml", &make_unique<data::detail::yaml_reader>);

    // writers
    auto twFactory {register_service<data::text_writer::factory>()};
    twFactory->add(".ini", &make_unique<data::detail::ini_writer>);
    twFactory->add(".json", &make_unique<data::detail::json_writer>);
    twFactory->add(".xml", &make_unique<data::detail::xml_writer>);
    twFactory->add(".yaml", &make_unique<data::detail::yaml_writer>);

    // binary
    // readers
    auto brFactory {register_service<data::binary_reader::factory>()};
    brFactory->add(".bsbd", &make_unique<data::detail::bsbd_reader>);

    // writers
    auto bwFactory {register_service<data::binary_writer::factory>()};
    bwFactory->add(".bsbd", &make_unique<data::detail::bsbd_writer>);
}

void platform::InitAssetFormats()
{
    auto factory {register_service<assets::loader_manager::factory>()};
    factory->add({".ini", ".json", ".xml", ".yaml"},
                 [](assets::group& group) {
                     return std::make_unique<detail::cfg_asset_loader_manager>(group);
                 });
}

void platform::InitImageCodecs()
{
    // decoders
    auto idFactory {register_service<gfx::image_decoder::factory>()};
    idFactory->add(".bmp", &make_unique<gfx::detail::bmp_decoder>);
    idFactory->add(".bsi", &make_unique<gfx::detail::bsi_decoder>);
    idFactory->add(".tga", &make_unique<gfx::detail::tga_decoder>);
    idFactory->add(".pcx", &make_unique<gfx::detail::pcx_decoder>);
    idFactory->add(".pnm", &make_unique<gfx::detail::pnm_decoder>);
    idFactory->add(".gif", &make_unique<gfx::detail::gif_decoder>);
    idFactory->add(".png", &make_unique<gfx::detail::png_decoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)
    idFactory->add(".qoi", &make_unique<gfx::detail::qoi_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    idFactory->add(".webp", &make_unique<gfx::detail::webp_decoder>);
#endif

    // encoders
    auto ieFactory {register_service<gfx::image_encoder::factory>()};
    ieFactory->add(".bmp", &make_unique<gfx::detail::bmp_encoder>);
    ieFactory->add(".bsi", &make_unique<gfx::detail::bsi_encoder>);
    ieFactory->add(".tga", &make_unique<gfx::detail::tga_encoder>);
    ieFactory->add(".pcx", &make_unique<gfx::detail::pcx_encoder>);
    ieFactory->add(".png", &make_unique<gfx::detail::png_encoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)
    ieFactory->add(".qoi", &make_unique<gfx::detail::qoi_encoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    ieFactory->add(".webp", &make_unique<gfx::detail::webp_encoder>);
#endif

    // animated
    // decoders
    auto iadFactory {register_service<gfx::animated_image_decoder::factory>()};
    iadFactory->add(".gif", &make_unique<gfx::detail::gif_decoder>);
    iadFactory->add(".png", &make_unique<gfx::detail::png_anim_decoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    iadFactory->add(".webp", &make_unique<gfx::detail::webp_anim_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_GFX_THEORA)
    iadFactory->add(".ogv", &make_unique<gfx::detail::theora_decoder>);
#endif

    // encoders
    auto iaeFactory {register_service<gfx::animated_image_encoder::factory>()};
    iaeFactory->add(".png", &make_unique<gfx::detail::png_anim_encoder>);
#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    iaeFactory->add(".webp", &make_unique<gfx::detail::webp_anim_encoder>);
#endif
}

void platform::InitAudioCodecs()
{
    // decoders
    auto adFactory {register_service<audio::decoder::factory>()};
    adFactory->add(".bsa", &make_unique<audio::detail::bsa_decoder>);
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)
    adFactory->add(".flac", &make_unique<audio::detail::flac_decoder>);
    adFactory->add(".mp3", &make_unique<audio::detail::mp3_decoder>);
    adFactory->add(".wav", &make_unique<audio::detail::wav_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_VORBIS)
    adFactory->add(".ogg", &make_unique<audio::detail::vorbis_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_OPUS)
    adFactory->add(".opus", &make_unique<audio::detail::opus_decoder>);
#endif
#if defined(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)
    adFactory->add(".mid", &make_unique<audio::detail::midi_decoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_LIBXMP)
    adFactory->add({".it", ".mod", ".s3m", ".xm"}, &make_unique<audio::detail::xmp_decoder>);
#endif

    // encoders
    auto aeFactory {register_service<audio::encoder::factory>()};
    aeFactory->add(".bsa", &make_unique<audio::detail::bsa_encoder>);
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)
    aeFactory->add(".wav", &make_unique<audio::detail::wav_encoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_VORBIS)
    aeFactory->add(".ogg", &make_unique<audio::detail::vorbis_encoder>);
#endif
#if defined(TCOB_ENABLE_FILETYPES_AUDIO_OPUS)
    aeFactory->add(".opus", &make_unique<audio::detail::opus_encoder>);
#endif
}

void platform::InitFontEngines()
{
    gfx::truetype_font_engine::Init();
}

void platform::InitTaskManager(std::optional<isize> workerThreads)
{
    register_service<task_manager>(std::make_shared<task_manager>(
        workerThreads
            ? *workerThreads
            : static_cast<isize>(std::thread::hardware_concurrency())));
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
