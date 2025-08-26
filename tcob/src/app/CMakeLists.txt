# app
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loaders/ConfigAssetLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/null/audio/NullAudioSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/null/input/NullInputSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/audio/SDLAudioStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/audio/SDLAudioSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/input/SDLInputEnums.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/input/SDLInputSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/SDLWindow.cpp
)

list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/app/Game.hpp
    ${TCOB_INC_DIR}/tcob/app/Game.inl
    ${TCOB_INC_DIR}/tcob/app/Platform.hpp
    ${TCOB_INC_DIR}/tcob/app/Scene.hpp
    ${TCOB_INC_DIR}/tcob/app/Scene.inl
)

if(TCOB_ENABLE_RENDERER_NULL)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/null/gfx/NullRenderSystem.cpp
    )
endif()

# OpenGL
if(TCOB_ENABLE_RENDERER_OPENGL45)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLCanvas.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLContext.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLEnum.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLFramebuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLObject.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLRenderSystem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLRenderTarget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLShaderProgram.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLTexture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLUniformBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLVertexArray.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gl45/GLWindow.cpp
    )
endif()

if(TCOB_ENABLE_RENDERER_OPENGLES30)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30Canvas.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30Context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30Enum.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30Framebuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30Object.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30RenderSystem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30RenderTarget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30ShaderProgram.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30Texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30UniformBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30VertexArray.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/SDL/gfx/gles30/GLES30Window.cpp
    )
endif()

tcob_add_obj_library(tcob_app "${SRC}" "${HDR}")
