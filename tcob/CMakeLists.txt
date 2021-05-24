add_subdirectory(extlibs)
###########################################################################

add_library(tcob OBJECT)

set(LIB_SRCFILES 
        src/assets/ResourceLibrary.cpp
        src/assets/Resource.cpp
        src/assets/loaders/AnimationLoader.cpp
        src/assets/loaders/CursorLoader.cpp
        src/assets/loaders/FontLoader.cpp
        src/assets/loaders/MaterialLoader.cpp
        src/assets/loaders/ParticleSystemLoader.cpp
        src/assets/loaders/ShaderLoader.cpp
        src/assets/loaders/TextureLoader.cpp
        src/assets/loaders/WebpAnimationLoader.cpp
        src/core/Automation.cpp
        src/core/Input.cpp
        src/core/Random.cpp
        src/core/Stopwatch.cpp
        src/core/data/Color.cpp
        src/core/data/Transform.cpp
        src/core/io/FileStream.cpp
        src/core/io/FileSystem.cpp
        src/core/io/Logger.cpp
        src/game/Config.cpp
        src/game/Game.cpp
        src/game/Scene.cpp
        src/game/ScriptApi.cpp
        src/game/script-api/CanvasApi.cpp
        src/gfx/Animation.cpp
        src/gfx/Camera.cpp
        src/gfx/Canvas.cpp
        src/gfx/Font.cpp
        src/gfx/Image.cpp
        src/gfx/ImageCodecs.cpp
        src/gfx/Quad.cpp
        src/gfx/TextFormatter.cpp
        src/gfx/Transformable.cpp
        src/gfx/drawables/Cursor.cpp
        src/gfx/drawables/Drawable.cpp
        src/gfx/drawables/WebpAnimation.cpp
        src/gfx/drawables/NinePatch.cpp
        src/gfx/drawables/ParticleSystem.cpp
        src/gfx/drawables/PointCloud.cpp
        src/gfx/drawables/Sprite.cpp
        src/gfx/drawables/Text.cpp
        src/gfx/drawables/TileMap.cpp
        src/gfx/gl/GLCapabilities.cpp
        src/gfx/gl/GLContext.cpp
        src/gfx/gl/GLEnum.cpp
        src/gfx/gl/GLFramebuffer.cpp
        src/gfx/gl/GLObject.cpp
        src/gfx/gl/GLObjectRegistry.cpp
        src/gfx/gl/GLPixelBuffer.cpp
        src/gfx/gl/GLRenderTarget.cpp
        src/gfx/gl/GLRenderTexture.cpp
        src/gfx/gl/GLRenderer.cpp
        src/gfx/gl/GLShaderProgram.cpp
        src/gfx/gl/GLShaderStorageBuffer.cpp
        src/gfx/gl/GLTexture.cpp
        src/gfx/gl/GLUniformBuffer.cpp
        src/gfx/gl/GLVertexArray.cpp
        src/gfx/gl/GLWindow.cpp
        src/script/LuaFunction.cpp
        src/script/LuaRef.cpp
        src/script/LuaScript.cpp
        src/script/LuaState.cpp
        src/script/LuaTable.cpp
        src/gfx/gl/nanovg/GLNVGcontext.cpp
)

set(LIB_HEADERFILES 
        include/tcob/assets/Resource.hpp
        include/tcob/assets/ResourceLibrary.hpp
        include/tcob/core/ConnectionManager.hpp
        include/tcob/core/Automation.hpp
        include/tcob/core/Helper.hpp
        include/tcob/core/Input.hpp
        include/tcob/core/Random.hpp
        include/tcob/core/Stopwatch.hpp
        include/tcob/core/data/Color.hpp
        include/tcob/core/data/Point.hpp
        include/tcob/core/data/Rect.hpp
        include/tcob/core/data/Size.hpp
        include/tcob/core/data/Transform.hpp
        include/tcob/core/io/FileStream.hpp
        include/tcob/core/io/FileSystem.hpp
        include/tcob/core/io/Logger.hpp
        include/tcob/game/Config.hpp
        include/tcob/game/Game.hpp
        include/tcob/game/Scene.hpp
        include/tcob/game/ScriptApi.hpp
        include/tcob/gfx/Animation.hpp
        include/tcob/gfx/Camera.hpp
        include/tcob/gfx/Canvas.hpp
        include/tcob/gfx/Font.hpp
        include/tcob/gfx/Image.hpp
        include/tcob/gfx/Material.hpp
        include/tcob/gfx/Quad.hpp
        include/tcob/gfx/TextFormatter.hpp
        include/tcob/gfx/Transformable.hpp
        include/tcob/gfx/drawables/Cursor.hpp
        include/tcob/gfx/drawables/Drawable.hpp
        include/tcob/gfx/drawables/WebpAnimation.hpp
        include/tcob/gfx/drawables/NinePatch.hpp
        include/tcob/gfx/drawables/ParticleSystem.hpp
        include/tcob/gfx/drawables/PointCloud.hpp
        include/tcob/gfx/drawables/Sprite.hpp
        include/tcob/gfx/drawables/Text.hpp
        include/tcob/gfx/drawables/TileMap.hpp
        include/tcob/gfx/gl/GLCapabilities.hpp
        include/tcob/gfx/gl/GLContext.hpp
        include/tcob/gfx/gl/GLEnum.hpp
        include/tcob/gfx/gl/GLFramebuffer.hpp
        include/tcob/gfx/gl/GLObject.hpp
        include/tcob/gfx/gl/GLObjectRegistry.hpp
        include/tcob/gfx/gl/GLPixelBuffer.hpp
        include/tcob/gfx/gl/GLRenderTarget.hpp
        include/tcob/gfx/gl/GLRenderTexture.hpp
        include/tcob/gfx/gl/GLRenderer.hpp
        include/tcob/gfx/gl/GLShaderProgram.hpp
        include/tcob/gfx/gl/GLShaderStorageBuffer.hpp
        include/tcob/gfx/gl/GLTexture.hpp
        include/tcob/gfx/gl/GLUniformBuffer.hpp
        include/tcob/gfx/gl/GLVertexArray.hpp
        include/tcob/gfx/gl/GLWindow.hpp
        include/tcob/script/LuaClosure.hpp
        include/tcob/script/LuaConversions.hpp
        include/tcob/script/LuaFunction.hpp
        include/tcob/script/LuaRef.hpp
        include/tcob/script/LuaScript.hpp
        include/tcob/script/LuaState.hpp
        include/tcob/script/LuaTable.hpp
        include/tcob/script/LuaWrapper.hpp
        include/tcob/tcob.hpp
        include/tcob/tcob_config.hpp

        include/tcob/thirdparty/sigslot/signal.hpp
)

target_sources(tcob PRIVATE ${LIB_SRCFILES} PRIVATE ${LIB_HEADERFILES})

target_include_directories(tcob PRIVATE ./include PRIVATE ./extlibs)

target_link_libraries(tcob PRIVATE tcob_extlibs 
        $<$<CXX_COMPILER_ID:MSVC>:opengl32>)

target_compile_options(tcob PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

add_library(tcob-static STATIC)
target_link_libraries(tcob-static PRIVATE tcob)