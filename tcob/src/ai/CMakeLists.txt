# ai
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Pathfinding.cpp
)

list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/ai/Pathfinding.hpp
)

tcob_add_obj_library(tcob_ai "${SRC}" "${HDR}")