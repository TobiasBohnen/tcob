add_executable(TestGame)

find_package(SDL2 CONFIG REQUIRED)

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

target_link_libraries(TestGame PRIVATE tcob PUBLIC SDL2::SDL2main)

target_include_directories(TestGame PRIVATE ../../../tcob/include)

set_target_properties(TestGame PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")