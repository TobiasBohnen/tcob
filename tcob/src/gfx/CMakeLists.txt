# gfx
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Animation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ColorGradient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_bmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_bsi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_gif.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_pcx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_png_interlaced.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_png_non_interlaced.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_pnm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_tga.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ImageFilters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Geometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QuadTween.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderTarget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderTexture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ShaderProgram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TextFormatter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Transformable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UniformBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VertexArray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drawables/Cursor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drawables/Drawable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drawables/ParticleSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drawables/PointCloud.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drawables/Shape.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drawables/Text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drawables/TileMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Accordion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Button.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/CanvasWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Checkbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/CycleButton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Displays.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/DropDownList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/GridView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/ImageBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Label.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/ListBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Panel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Progressbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Radiobutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Slider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Spinner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/TabContainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Terminal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/TextBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Toggle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Tooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/VScrollWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/Widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets/WidgetContainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Form.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Layout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Style.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/UI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/WidgetPainter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/WidgetTweener.cpp
)

list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/gfx/Animation.hpp
    ${TCOB_INC_DIR}/tcob/gfx/Camera.hpp
    ${TCOB_INC_DIR}/tcob/gfx/Canvas.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ColorGradient.hpp
    ${TCOB_INC_DIR}/tcob/gfx/Gfx.hpp
    ${TCOB_INC_DIR}/tcob/gfx/Font.hpp
    ${TCOB_INC_DIR}/tcob/gfx/Image.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ImageFilters.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ImageFilters.inl
    ${TCOB_INC_DIR}/tcob/gfx/Material.hpp
    ${TCOB_INC_DIR}/tcob/gfx/Geometry.hpp
    ${TCOB_INC_DIR}/tcob/gfx/QuadTween.hpp
    ${TCOB_INC_DIR}/tcob/gfx/QuadTween.inl
    ${TCOB_INC_DIR}/tcob/gfx/Renderer.hpp
    ${TCOB_INC_DIR}/tcob/gfx/RenderSystem.hpp
    ${TCOB_INC_DIR}/tcob/gfx/RenderSystemImpl.hpp
    ${TCOB_INC_DIR}/tcob/gfx/RenderTarget.hpp
    ${TCOB_INC_DIR}/tcob/gfx/RenderTarget.inl
    ${TCOB_INC_DIR}/tcob/gfx/RenderTexture.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ShaderProgram.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ShaderProgram.inl
    ${TCOB_INC_DIR}/tcob/gfx/TextFormatter.hpp
    ${TCOB_INC_DIR}/tcob/gfx/Texture.hpp
    ${TCOB_INC_DIR}/tcob/gfx/Texture.inl
    ${TCOB_INC_DIR}/tcob/gfx/Transformable.hpp
    ${TCOB_INC_DIR}/tcob/gfx/UniformBuffer.hpp
    ${TCOB_INC_DIR}/tcob/gfx/VertexArray.hpp
    ${TCOB_INC_DIR}/tcob/gfx/Window.hpp
    ${TCOB_INC_DIR}/tcob/gfx/drawables/Cursor.hpp
    ${TCOB_INC_DIR}/tcob/gfx/drawables/Drawable.hpp
    ${TCOB_INC_DIR}/tcob/gfx/drawables/ParticleSystem.hpp
    ${TCOB_INC_DIR}/tcob/gfx/drawables/PointCloud.hpp
    ${TCOB_INC_DIR}/tcob/gfx/drawables/Shape.hpp
    ${TCOB_INC_DIR}/tcob/gfx/drawables/Shape.inl
    ${TCOB_INC_DIR}/tcob/gfx/drawables/Text.hpp
    ${TCOB_INC_DIR}/tcob/gfx/drawables/TileMap.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Accordion.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Accordion.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Button.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/CanvasWidget.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Checkbox.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/CycleButton.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Displays.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/DropDownList.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/GridView.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/ImageBox.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Label.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/ListBox.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Panel.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Panel.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Progressbar.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Radiobutton.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Slider.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Spinner.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/TabContainer.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/TabContainer.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Terminal.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/TextBox.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Toggle.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Tooltip.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/VScrollWidget.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Widget.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/Widget.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/WidgetContainer.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/widgets/WidgetContainer.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/Form.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/Form.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/Layout.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/Layout.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/Scrollbar.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/Scrollbar.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/Style.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/Style.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/UI.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/UI.inl
    ${TCOB_INC_DIR}/tcob/gfx/ui/WidgetPainter.hpp
    ${TCOB_INC_DIR}/tcob/gfx/ui/WidgetTweener.hpp
)

# litehtml
if(TCOB_ENABLE_ADDON_GFX_LITEHTML)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/html/HtmlDocument.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/html/HtmlContainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/html/HtmlElementPainter.cpp
    )
    list(APPEND HDR
        ${TCOB_INC_DIR}/tcob/gfx/html/HtmlDocument.hpp
        ${TCOB_INC_DIR}/tcob/gfx/html/HtmlElementPainter.hpp
    )
endif()

# OpenGL
if(TCOB_ENABLE_RENDERER_OPENGL45)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLContext.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLEnum.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLFramebuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLObject.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLRenderSystem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLRenderTarget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLShaderProgram.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLTexture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLUniformBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLVertexArray.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/GLWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gl45/nanovg/GLCanvas.cpp
    )
endif()

if(TCOB_ENABLE_RENDERER_OPENGLES30)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESContext.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESEnum.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESFramebuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESObject.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESRenderSystem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESRenderTarget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESShaderProgram.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESTexture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESUniformBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESVertexArray.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/GLESWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/gles30/nanovg/GLESCanvas.cpp
    )
endif()

if(TCOB_ENABLE_RENDERER_NULL)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/null/NullRenderSystem.cpp
    )
endif()

if(TCOB_ENABLE_FILETYPES_GFX_QOI)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_qoi.cpp
    )
endif()

if(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_webp.cpp
    )
endif()

if(TCOB_ENABLE_FILETYPES_GFX_THEORA)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/image_codecs/ImageCodec_theora.cpp
    )
endif()

tcob_add_obj_library(tcob_gfx "${SRC}" "${HDR}")