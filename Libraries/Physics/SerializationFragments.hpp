///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

inline void SerializeAnchors(Serializer& stream, AnchorAtom& anchor, Vec3Param defaults = Vec3(1,1,1))
{
  stream.SerializeFieldDefault("LocalPointA", anchor[0], defaults);
  stream.SerializeFieldDefault("LocalPointB", anchor[1], defaults);
}

inline void SerializeAxes(Serializer& stream, AxisAtom& axes, Vec3Param defaults = Vec3::cYAxis)
{
  stream.SerializeFieldDefault("LocalAxisA", axes[0], defaults);
  stream.SerializeFieldDefault("LocalAxisB", axes[1], defaults);
}

inline void SerializeAngles(Serializer& stream, AngleAtom& angles)
{
  stream.SerializeFieldDefault("LocalAngleA", angles[0], Quat::cIdentity);
  stream.SerializeFieldDefault("LocalAngleB", angles[1], Quat::cIdentity);
}

}//namespace Physics

}//namespace Zero
