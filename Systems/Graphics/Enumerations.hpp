#pragma once

namespace Zero
{

/// How pixel values (source) are combined with the active render target's values (destination).
/// <param name="Add">Source is added to destination.</param>
/// <param name="Subtract">Source is subtracted from destination.</param>
/// <param name="ReverseSubtract">Destination is subtracted from source.</param>
/// <param name="Min">The minimum value between source and destination is kept.</param>
/// <param name="Max">The maximum value between source and destination is kept.</param>
DeclareEnum5(BlendEquation, Add, Subtract, ReverseSubtract, Min, Max);

/// Values that can be multiplied with the operands used in the blend equation.
/// <param name="Zero">Multiplied by zero, operand results in zero.</param>
/// <param name="One">Multiplied by one, operand is unchanged.</param>
/// <param name="SourceColor">The RGB of the source value.</param>
/// <param name="InvSourceColor">One minus the RGB of the source value.</param>
/// <param name="DestColor">The RGB of the destination value.</param>
/// <param name="InvDestColor">One minus the RGB of the destination value.</param>
/// <param name="SourceAlpha">The alpha channel of the source value.</param>
/// <param name="InvSourceAlpha">One minus the alpha channel of the source value.</param>
/// <param name="DestAlpha">The alpha channel of the destination value.</param>
/// <param name="InvDestAlpha">One minus the alpha channel of the destination value.</param>
/// <param name="SourceAlphaSaturate">Minimum of source alpha and one minus destination alpha.</param>
DeclareEnum11(BlendFactor, Zero, One, SourceColor, InvSourceColor, DestColor, InvDestColor, SourceAlpha, InvSourceAlpha, DestAlpha, InvDestAlpha, SourceAlphaSaturate);

/// If blend equations are applied when writing to a render target.
/// <param name="Disabled">Blending is not used.</param>
/// <param name="Enabled">Blending is used, color and alpha use the same equations.</param>
/// <param name="Separate">Blending is used, color and alpha use separate equations.</param>
DeclareEnum3(BlendMode, Disabled, Enabled, Separate);

DeclareEnum4(CoreVertexType, Mesh, SkinnedMesh, Streamed, Count);

/// How triangles should be culled (not rendered) depending on which way they face.
/// <param name="Disabled">Triangles are always rendered.</param>
/// <param name="BackFace">Clockwise triangles are not rendered.</param>
/// <param name="FrontFace">Counter clockwise triangles are not rendered.</param>
DeclareEnum3(CullMode, Disabled, BackFace, FrontFace);

/// How pixels depth values should be tested to determine if the pixel renders.
/// <param name="Disabled">No depth testing.</param>
/// <param name="Read">Depth values are compared to the depth buffer.</param>
/// <param name="Write">Depth values are compared to the depth buffer, if comparison succeeds the new value is written to the buffer.</param>
DeclareEnum3(DepthMode, Disabled, Read, Write);

/// Method of sorting for determining the draw order of graphicals.
/// <param name="None">Not sorted, draw order is undefined.</param>
/// <param name="GraphicalSortValue">Uses the sort value on the graphical component, lowest to highest.</param>
/// <param name="SortEvent">An event is sent on CameraViewport every frame for custom logic to determine the sort value.</param>
/// <param name="BackToFrontView">Uses the distance in the view direction, furthest to nearest.</param>
/// <param name="FrontToBackView">Uses the distance in the view direction, nearest to furthest.</param>
/// <param name="NegativeToPositiveX">Uses the world space X-coordinate.</param>
/// <param name="PositiveToNegativeX">Uses the world space X-coordinate.</param>
/// <param name="NegativeToPositiveY">Uses the world space Y-coordinate.</param>
/// <param name="PositiveToNegativeY">Uses the world space Y-coordinate.</param>
/// <param name="NegativeToPositiveZ">Uses the world space Z-coordinate.</param>
/// <param name="PositiveToNegativeZ">Uses the world space Z-coordinate.</param>
DeclareEnum11(GraphicalSortMethod, None, GraphicalSortValue, SortEvent, BackToFrontView, FrontToBackView, NegativeToPositiveX, PositiveToNegativeX, NegativeToPositiveY, PositiveToNegativeY, NegativeToPositiveZ, PositiveToNegativeZ);

/// Method of 3D scene projection to a 2D plane.
/// <param name="Perspective">Projection towards a single view point, making distant objects appear smaller.</param>
/// <param name="Orthographic">Projection is parallel with the viewing direction.</param>
DeclareEnum2(PerspectiveMode, Perspective, Orthographic);

DeclareEnum2(RenderingType, Static, Streamed);

DeclareEnum5(RenderTaskType, ClearTarget, RenderPass, PostProcess, BackBufferBlit, TextureUpdate);

DeclareEnum2(ScissorMode, Disabled, Enabled);

DeclareEnum14(ShaderInputType, Invalid, Bool, Int, IntVec2, IntVec3, IntVec4, Float, Vec2, Vec3, Vec4, Mat3, Mat4, Texture, Count);

/// How the sprite quad is aligned in 3D space.
/// <param name="ZPlane">Aligned with the object's z axis.</param>
/// <param name="ViewPlane">Aligned to always face the camera.</param>
DeclareEnum2(SpriteGeometryMode, ZPlane, ViewPlane);

/// How stencil buffer values should be tested to determine if a pixel renders.
/// <param name="Disabled">No stencil testing.</param>
/// <param name="Enabled">Stencil values will be tested with the active settings.</param>
/// <param name="Separate">Stencil values will be tested, frontface/backface triangles will use separate settings.</param>
DeclareEnum3(StencilMode, Disabled, Enabled, Separate);

/// Operations that can be applied to the stencil buffer.
/// <param name="Zero">Sets the stencil buffer value to 0.</param>
/// <param name="Keep">Keeps the current value.</param>
/// <param name="Replace">Sets the stencil buffer value to the test value.</param>
/// <param name="Invert">Bitwise inverts the current stencil buffer value.</param>
/// <param name="Increment">Increments the current stencil buffer value. Clamps to the maximum value.</param>
/// <param name="IncrementWrap">Increments the current stencil buffer value. Wraps stencil buffer value to zero when incrementing the maximum value.</param>
/// <param name="Decrement">Decrements the current stencil buffer value. Clamps to 0.</param>
/// <param name="DecrementWrap">Decrements the current stencil buffer value. Wraps stencil buffer value to the maximum value when decrementing a stencil buffer value of zero.</param>
DeclareEnum8(StencilOp, Zero, Keep, Replace, Invert, Increment, IncrementWrap, Decrement, DecrementWrap);

/// How text is positioned relative to the object position.
/// <param name="Left">Left side is aligned with the position.</param>
/// <param name="Center">Center is aligned with the position.</param>
/// <param name="Right">Right side is aligned with the position.</param>
DeclareEnum3(TextAlign, Left, Center, Right);

DeclareEnum2(TextRounding, Nearest, LastCharacter);

/// Method of comparison used when testing values against textures/buffers.
/// <param name="Never">Compare never succeeds.</param>
/// <param name="Always">Compare always succeeds.</param>
/// <param name="Less">Compares if test value is less than buffer value.</param>
/// <param name="LessEqual">Compares if test value is less than or equal to buffer value.</param>
/// <param name="Greater">Compares if test value is greater than buffer value.</param>
/// <param name="GreaterEqual">Compares if test value is greater than or equal to buffer value.</param>
/// <param name="Equal">Compares if test value is equal to buffer value.</param>
/// <param name="NotEqual">Compares if test value is not equal to buffer value.</param>
DeclareEnum8(TextureCompareFunc, Never, Always, Less, LessEqual, Greater, GreaterEqual, Equal, NotEqual);

/// If texture sampling should do comparison.
/// <param name="Disabled">Regular texture sampling.</param>
/// <param name="Enabled">Sampling returns the result of a comparison.</param>
DeclareEnum2(TextureCompareMode, Disabled, Enabled);

/// How the viewport should be sized within the window size.
/// <param name="Fill">Viewport fills the whole size.</param>
/// <param name="Letterbox">Viewport aspect ratio is always preserved.</param>
/// <param name="Exact">Viewport is not scaled.</param>
/// <param name="LargestMultiple">Viewport is the largest multiple of the set resolution that fits in the window.</param>
DeclareEnum4(ViewportScaling, Fill, Letterbox, Exact, LargestMultiple);

} // namespace Zero
