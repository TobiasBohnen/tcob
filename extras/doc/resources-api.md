# **Resources API**

- [**Resources API**](#resources-api)
  - [**tcob::FrameAnimation**](#tcobframeanimation)
  - [**tcob::Cursor**](#tcobcursor)
  - [**tcob::Font**](#tcobfont)
  - [**tcob::Material**](#tcobmaterial)
  - [**tcob::Music**](#tcobmusic)
  - [**tcob::ParticleSystem**](#tcobparticlesystem)
  - [**tcob::Sound**](#tcobsound)
  - [**tcob::gl::ShaderProgram**](#tcobglshaderprogram)
  - [**tcob::gl::Texture**](#tcobgltexture)
  - [**tcob::WebpAnimation**](#tcobwebpanimation)
  
## **tcob::FrameAnimation**

- animation [name]
  - frames { [texture::regions::name] }
  - duration [int]
  - playback_mode [AnimationPlaybackMode]

## **tcob::Cursor**

- cursor [name]
  - material [material::name]
  - modes
    - { [name] = { texture = [file], hotspot = [PointI] } }
  
## **tcob::Font**

- font [name]
  - source [file]
  - size [int]
  - kerning [bool]
  - is_default [bool]
  - material [material::name]
  - line_gap [float]

- sdf_font [name]
  - source [file]
  - size [int]
  - kerning [bool]
  - is_default [bool]
  - material [material::name]
  - line_gap [float]

## **tcob::Material**

- material [name]
  - texture [texture::name]
  - shader [shader::name]
  - blend_func
  - separate_blend_func
  - blend_equation

## **tcob::Music**

- music [name]
  - source [file]

## **tcob::ParticleSystem**

- particle_system [name]
  - material [material::name]
  - emitters { [particle_emitter::name] }

- particle_emitter [name]
  - spawnarea [RectF]
  - lifetime [float]
  - loop [bool]
  - spawnrate [float]
  - texture [texture::name]
  - template [particle_template::name]
  
- particle_template [name]
  - direction [float, float]
  - speed [float, float]
  - acceleration [float, float]
  - size [SizeF]
  - scale [float, float]
  - spin [float, float]
  - lifetime [float, float]
  - transparency [float, float]

## **tcob::Sound**

- sound [name]
  - source [file]

## **tcob::gl::ShaderProgram**

- shader [name]
  - fragment [file]
  - vertex [file]
  - default_for { 'UI', 'Window', 'Font' }

## **tcob::gl::Texture**

- texture [name]
  - source [file]
  - regions
    - { [name] = { height, width, left, top }  }
  - filtering
  - wrapping

- texture_array [name]
  - source { [file|folder] }
  - filtering
  - wrapping

## **tcob::WebpAnimation**

- webp-animation [name]
  - source [file]
  - material [material::name]
