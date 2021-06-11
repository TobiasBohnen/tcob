// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/assets/Resource.hpp>
#include <tcob/assets/ResourceLibrary.hpp>

#include <tcob/core/io/FileStream.hpp>
#include <tcob/core/io/FileSystem.hpp>
#include <tcob/core/io/Logger.hpp>

#include <tcob/core/data/Color.hpp>
#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/core/data/Size.hpp>
#include <tcob/core/data/Transform.hpp>

#include <tcob/core/Automation.hpp>
#include <tcob/core/ConnectionManager.hpp>
#include <tcob/core/Helper.hpp>
#include <tcob/core/Input.hpp>
#include <tcob/core/Random.hpp>
#include <tcob/core/Stopwatch.hpp>
#include <tcob/core/Timer.hpp>

#include <tcob/game/Config.hpp>
#include <tcob/game/Game.hpp>
#include <tcob/game/Scene.hpp>
#include <tcob/game/ScriptApi.hpp>

#include <tcob/gfx/drawables/Cursor.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/drawables/NinePatch.hpp>
#include <tcob/gfx/drawables/ParticleSystem.hpp>
#include <tcob/gfx/drawables/PointCloud.hpp>
#include <tcob/gfx/drawables/Sprite.hpp>
#include <tcob/gfx/drawables/Text.hpp>
#include <tcob/gfx/drawables/TileMap.hpp>
#include <tcob/gfx/drawables/WebpAnimation.hpp>

#include <tcob/gfx/gl/GLCapabilities.hpp>
#include <tcob/gfx/gl/GLContext.hpp>
#include <tcob/gfx/gl/GLEnum.hpp>
#include <tcob/gfx/gl/GLFramebuffer.hpp>
#include <tcob/gfx/gl/GLObject.hpp>
#include <tcob/gfx/gl/GLObjectRegistry.hpp>
#include <tcob/gfx/gl/GLPixelBuffer.hpp>
#include <tcob/gfx/gl/GLRenderTarget.hpp>
#include <tcob/gfx/gl/GLRenderTexture.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>
#include <tcob/gfx/gl/GLShaderProgram.hpp>
#include <tcob/gfx/gl/GLShaderStorageBuffer.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>
#include <tcob/gfx/gl/GLUniformBuffer.hpp>
#include <tcob/gfx/gl/GLVertexArray.hpp>
#include <tcob/gfx/gl/GLWindow.hpp>

#include <tcob/gfx/Animation.hpp>
#include <tcob/gfx/Camera.hpp>
#include <tcob/gfx/Canvas.hpp>
#include <tcob/gfx/Font.hpp>
#include <tcob/gfx/Image.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/Transformable.hpp>

#include <tcob/script/LuaScript.hpp>

#include <tcob/sfx/Music.hpp>
#include <tcob/sfx/Sound.hpp>
