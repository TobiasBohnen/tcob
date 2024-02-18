# app
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loaders/ConfigAssetLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loaders/RasterFontLoader.cpp
)

list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/app/Game.hpp
    ${TCOB_INC_DIR}/tcob/app/Game.inl
    ${TCOB_INC_DIR}/tcob/app/Platform.hpp
    ${TCOB_INC_DIR}/tcob/app/Scene.hpp
    ${TCOB_INC_DIR}/tcob/app/Scene.inl
)

tcob_add_obj_library(tcob_app "${SRC}" "${HDR}")
