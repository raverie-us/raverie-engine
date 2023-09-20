// MIT Licensed (see LICENSE.md).
#pragma once

#include "Serialization/SerializationTraits.hpp"
#include "Foundation/Geometry/Polygon.hpp"
#include "Foundation/Geometry/Shape2D.hpp"

// Polygon and Shape2D are in Geometry which doesn't know about
// serialization. To deal with this we define a policy externally.
namespace Raverie
{

namespace Serialization
{

template <>
struct Policy<Polygon>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Polygon& value)
  {
    bool started = stream.Start("Polygon", fieldName, Trait<Polygon>::Type);
    if (started)
    {
      // serialize each member here
      stream.SerializeField("Data", value.mData);

      stream.End("Polygon", Trait<Polygon>::Type);
    }
    return started;
  }
};

template <>
struct Policy<Shape2D>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Shape2D& value)
  {
    bool started = stream.Start("Shape2D", fieldName, Trait<Shape2D>::Type);
    if (started)
    {
      // serialize each member here
      stream.SerializeField("Contours", value.mContours);

      stream.End("Shape2D", Trait<Shape2D>::Type);
    }
    return started;
  }
};

} // namespace Serialization

} // namespace Raverie
