# physics
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics.cpp
)
list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/physics/Physics.hpp
)

# Box2D
if(TCOB_ENABLE_ADDON_PHYSICS_BOX2D)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/B2D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Body.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DebugDraw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Joint.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Shape.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/World.cpp
    )

    list(APPEND HDR
        ${TCOB_INC_DIR}/tcob/physics/Body.hpp
        ${TCOB_INC_DIR}/tcob/physics/DebugDraw.hpp
        ${TCOB_INC_DIR}/tcob/physics/Joint.hpp
        ${TCOB_INC_DIR}/tcob/physics/Shape.hpp
        ${TCOB_INC_DIR}/tcob/physics/World.hpp
    )
endif()

tcob_add_obj_library(tcob_physics "${SRC}" "${HDR}")