add_executable(tcob_tests)

target_sources(tcob_tests PRIVATE  
    TextFormatterTests.cpp       
    AnimationTests.cpp
    ColorTests.cpp
    FileSystemTests.cpp
    LuaScriptTests.cpp
    LuaWrapperTests.cpp
    PointTests.cpp
    RandomTests.cpp
    RectTests.cpp
    SpriteBatchTests.cpp
    AutomationTests.cpp
    main.cpp
    LuaScriptTestsClass.hpp
    LuaScriptTestsHelper.hpp
    catch.hpp
    tests.hpp
)

target_link_libraries(tcob_tests tcob)

target_include_directories(tcob_tests PRIVATE . PRIVATE ../tcob/include)