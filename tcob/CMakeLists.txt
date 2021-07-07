add_subdirectory(extlibs)
###########################################################################

add_library(tcob OBJECT)

set(LIB_SRCFILES 
        src/assets/ResourceLibrary.cpp
        src/assets/Resource.cpp
        src/assets/loaders/AnimationLoader.cpp
        src/assets/loaders/AudioLoader.cpp
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
        src/core/Timer.cpp
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
        src/gfx/QuadEffect.cpp
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
        src/sfx/ALObjects.cpp
        src/sfx/AudioCodecs.cpp
        src/sfx/AudioIO.cpp
        src/sfx/AudioSource.cpp
        src/sfx/AudioSystem.cpp
        src/sfx/Music.cpp
        src/sfx/Sound.cpp
        src/gfx/gl/nanovg/GLNVGcontext.cpp
)

target_sources(tcob PRIVATE ${LIB_SRCFILES})

target_include_directories(tcob PRIVATE ./include PRIVATE ./extlibs)

target_link_libraries(tcob PRIVATE tcob_extlibs 
        $<$<CXX_COMPILER_ID:MSVC>:opengl32>
)

target_compile_options(tcob PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

target_precompile_headers(tcob PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:tcob/pch.hpp>
)

add_library(tcob-static STATIC)
target_link_libraries(tcob-static PRIVATE tcob)