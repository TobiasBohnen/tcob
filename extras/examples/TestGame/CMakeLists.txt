add_executable(TestGame)

target_sources(TestGame PRIVATE 
    main.cpp 
    StartScene.cpp
    Scenes/TemplateScene.cpp
    Scenes/AutomationEx.cpp
    Scenes/CanvasEx.cpp
    Scenes/ControllerEx.cpp
    Scenes/TextEx.cpp
    Scenes/Misc.cpp 
)

target_link_libraries(TestGame PRIVATE tcob PUBLIC SDL2main)

if(MSVC)
    target_link_options(TestGame PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/icon.res)
endif()

target_include_directories(TestGame PRIVATE ../../../tcob/include)

set_target_properties(TestGame PROPERTIES 
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    INTERPROCEDURAL_OPTIMIZATION TRUE)
