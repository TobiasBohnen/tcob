# audio
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AudioSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Effect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Music.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Playlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Recording.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/synth/SoundGenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/synth/SoundGenerator_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/synth/SpeechGenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/audio_codecs/AudioCodec_bsa.cpp
)

list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/audio/Audio.hpp
    ${TCOB_INC_DIR}/tcob/audio/AudioSystem.hpp
    ${TCOB_INC_DIR}/tcob/audio/Buffer.hpp
    ${TCOB_INC_DIR}/tcob/audio/Effect.hpp
    ${TCOB_INC_DIR}/tcob/audio/Music.hpp
    ${TCOB_INC_DIR}/tcob/audio/Playlist.hpp
    ${TCOB_INC_DIR}/tcob/audio/Recording.hpp
    ${TCOB_INC_DIR}/tcob/audio/Sound.hpp
    ${TCOB_INC_DIR}/tcob/audio/Source.hpp
    ${TCOB_INC_DIR}/tcob/audio/synth/SoundGenerator.hpp
    ${TCOB_INC_DIR}/tcob/audio/synth/SpeechGenerator.hpp
)

if(TCOB_ENABLE_FILETYPES_AUDIO_DRLIBS)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/audio_codecs/AudioCodec_flac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/audio_codecs/AudioCodec_mp3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/audio_codecs/AudioCodec_wav.cpp
    )
endif()

if(TCOB_ENABLE_FILETYPES_AUDIO_VORBIS)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/audio_codecs/AudioCodec_ogg.cpp
    )
endif()

if(TCOB_ENABLE_FILETYPES_AUDIO_OPUS)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/audio_codecs/AudioCodec_opus.cpp
    )
endif()

if(TCOB_ENABLE_FILETYPES_AUDIO_LIBXMP)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/audio_codecs/AudioCodec_xmp.cpp
    )
endif()

# TinySoundFont
if(TCOB_ENABLE_ADDON_AUDIO_TINYSOUNDFONT)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/audio_codecs/AudioCodec_mid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/synth/SoundFont.cpp
    )
    list(APPEND HDR
        ${TCOB_INC_DIR}/tcob/audio/synth/SoundFont.hpp
        ${TCOB_INC_DIR}/tcob/audio/synth/SoundFont.inl
    )
endif()

tcob_add_obj_library(tcob_audio "${SRC}" "${HDR}")