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
        ${CMAKE_CURRENT_SOURCE_DIR}/box2d/B2D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/box2d/B2DBody.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/box2d/B2DContactListener.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/box2d/B2DDebugDraw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/box2d/B2DFixture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/box2d/B2DJoint.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/box2d/B2DShape.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/box2d/B2DWorld.cpp
    )

    list(APPEND HDR
        ${TCOB_INC_DIR}/tcob/physics/box2d/B2D.hpp
        ${TCOB_INC_DIR}/tcob/physics/box2d/B2DBody.hpp
        ${TCOB_INC_DIR}/tcob/physics/box2d/B2DDebugDraw.hpp
        ${TCOB_INC_DIR}/tcob/physics/box2d/B2DFixture.hpp
        ${TCOB_INC_DIR}/tcob/physics/box2d/B2DJoint.hpp
        ${TCOB_INC_DIR}/tcob/physics/box2d/B2DShape.hpp
        ${TCOB_INC_DIR}/tcob/physics/box2d/B2DWorld.hpp
    )
endif()

# Chipmunk2D
if(TCOB_ENABLE_ADDON_PHYSICS_CHIPMUNK2D)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/chipmunk2d/CP.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/chipmunk2d/CPBody.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/chipmunk2d/CPConstraint.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/chipmunk2d/CPShape.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/chipmunk2d/CPSpace.cpp
    )
    list(APPEND HDR
        ${TCOB_INC_DIR}/tcob/physics/chipmunk2d/CP.hpp
        ${TCOB_INC_DIR}/tcob/physics/chipmunk2d/CPBody.hpp
        ${TCOB_INC_DIR}/tcob/physics/chipmunk2d/CPConstraint.hpp
        ${TCOB_INC_DIR}/tcob/physics/chipmunk2d/CPShape.hpp
        ${TCOB_INC_DIR}/tcob/physics/chipmunk2d/CPSpace.hpp
    )
endif()

tcob_add_obj_library(tcob_physics "${SRC}" "${HDR}")