# **Resources API**

- [**Resources API**](#resources-api)
  - [**animation**](#animation)
  - [**cursor**](#cursor)
  - [**fonts**](#fonts)
  - [**material**](#material)
  - [**music**](#music)
  - [**particle_system**](#particle_system)
  - [**sound**](#sound)
  - [**shader**](#shader)
  - [**textures**](#textures)
  - [**webp-animation**](#webp-animation)
  
## **animation**

- animation [name]
  - frames { [texture::regions::name] }
  - duration [int]
  - playback_mode [AnimationPlaybackMode]

## **cursor**

- cursor [name]
  - material [material::name]
  - modes
    - { [name] = { texture = [file], hotspot = [PointI] } }
  
## **fonts**

- font [name]
  - source = [file]
  - size = [int]
  - kerning = [bool]
  - is_default
  - material [material::name]

- sdf_font [name]
  - source = [file]
  - size = [int]
  - kerning = [bool]
  - is_default
  - material [material::name]

## **material**

- material [name]
  - texture [texture::name]
  - shader [shader::name]
  - blend_func
  - separate_blend_func
  - blend_equation

## **music**

- music [name]
  - source [file]

## **particle_system**

- particle_system [name]
  - material [material::name]
  - emitters { [name] }

- particle_emitter [name]
  - spawnarea [RectF]
  - lifetime [float]
  - loop [bool]
  - spawnrate [float]
  - texture [texture::name]
  - template [name]
  
- particle_template [name]
  - direction [float, float]
  - speed [float, float]
  - acceleration [float, float]
  - size [SizeF]
  - scale [float, float]
  - spin [float, float]
  - lifetime [float, float]
  - transparency [float, float]

## **sound**

- sound [name]
  - source [file]

## **shader**

- shader [name]
  - fragment [file]
  - vertex [file]
  - default_for { 'UI', 'Window', 'Font' }

## **textures**

- texture [name]
  - source [file]
  - regions
    - { [name] = { height, width, left, top }  }
  - auto_regions { tilesize, padding }
  - filtering
  - wrapping

- texture_array [name]
  - source { [file|folder] }
  - filtering
  - wrapping

## **webp-animation**

- webp-animation [name]
  - source [file]
  - material [material::name]
