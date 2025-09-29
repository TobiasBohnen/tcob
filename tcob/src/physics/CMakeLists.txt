# physics
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/B2D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/B2DBody.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/B2DDebugDraw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/B2DJoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/B2DShape.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/B2DWorld.cpp
)
list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/physics/Physics.hpp
    ${TCOB_INC_DIR}/tcob/physics/B2DBody.hpp
    ${TCOB_INC_DIR}/tcob/physics/B2DDebugDraw.hpp
    ${TCOB_INC_DIR}/tcob/physics/B2DJoint.hpp
    ${TCOB_INC_DIR}/tcob/physics/B2DShape.hpp
    ${TCOB_INC_DIR}/tcob/physics/B2DWorld.hpp
)

tcob_add_obj_library(tcob_physics "${SRC}" "${HDR}")