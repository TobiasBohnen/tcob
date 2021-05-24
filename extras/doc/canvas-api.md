# **Canvas API**

- [**Canvas API**](#canvas-api)
  - [**Frame**](#frame)
  - [**Transforms**](#transforms)
  - [**State**](#state)
  - [**Paths**](#paths)
  - [**Shapes**](#shapes)
  - [**Render styles**](#render-styles)
  - [**Scissoring**](#scissoring)
  - [**Image**](#image)
  - [**Font**](#font)
  - [**Render target**](#render-target)
  - [**Enums**](#enums)
    - [Winding](#winding)
    - [Solidity](#solidity)
    - [LineCap](#linecap)
    - [LineJoin](#linejoin)
    - [Align](#align)
  
## **Frame**

- begin_frame
- end_frame

## **Transforms**

- translate
- rotate
- rotate_at
- scale
- scale_at
- reset_transform
  
## **State**

- save
- restore

## **Paths**

- PathWinding
- move_to
- line_to
- arc_to
- cubic_bezier_to
- quad_bezier_to
- begin_path
- close_path
- fill
- stroke
  
## **Shapes**

- fill_rect
- stroke_rect
- fill_rounded_rect
- stroke_rounded_rect
- fill_rounded_rect_varying
- stroke_rounded_rect_varying
- fill_circle
- stroke_circle
- fill_ellipse
- stroke_ellipse
- fill_arc
- stroke_arc
- fill_lines
- stroke_lines

## **Render styles**

- FillStyle
- StrokeStyle
- StrokeWidth
- GlobalAlpha
- ShapeAntiAlias
- MiterLimit
- LineCap
- LineJoin

## **Scissoring**

- Scissor
- intersect_scissor
- reset_scissor

## **Image**

- create_image
- draw_image

## **Font**

- create_font
- FontFace
- FontSize
- FontBlur
- TextAlign
- draw_text
- draw_textbox

## **Render target**

- WindowSize

## **Enums**

### Winding

- Winding::CW
- Winding::CCW

### Solidity

- Solidity::Solid
- Solidity::Hole

### LineCap

- LineCap::Butt
- LineCap::Round
- LineCap::Square

### LineJoin

- LineJoin::Round
- LineJoin::Bevel
- LineJoin::Miter

### Align

- Align::Left
- Align::Center
- Align::Right
- Align::Top
- Align::Middle
- Align::Bottom
- Align::Baseline
