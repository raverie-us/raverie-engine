// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineExternalBaseType(Ray, TypeCopyMode::ValueType, builder, type)
{
  RaverieBindDefaultCopyDestructor();
  RaverieFullBindConstructor(builder, type, RaverieSelf, "start, direction", Vec3Param, Vec3Param);

  RaverieBindMemberProperty(Start);
  RaverieBindMemberProperty(Direction);
  RaverieFullBindMethod(builder, type, &Ray::GetPoint, RaverieNoOverload, "GetPoint", "tValue")->Description = RaverieDocumentString("Returns the point at the given t-value.");
  RaverieFullBindMethod(builder, type, &Ray::GetTValue, RaverieNoOverload, "GetTValue", "point")->Description = RaverieDocumentString("Returns the t-value that would result in the given "
                                                                                                                                      "point projected onto the ray.");
  type->ToStringFunction = Raverie::BoundTypeToGlobalToString<Ray>;
  type->AddAttribute(ExportDocumentation);
}

RaverieDefineExternalBaseType(Segment, TypeCopyMode::ValueType, builder, type)
{
  RaverieBindDefaultCopyDestructor();
  RaverieFullBindConstructor(builder, type, RaverieSelf, "start, end", Vec3Param, Vec3Param);

  RaverieBindMemberProperty(Start);
  RaverieBindMemberProperty(End);
  RaverieFullBindMethod(builder, type, &Segment::GetPoint, RaverieNoOverload, "GetPoint", "tValue")->Description = RaverieDocumentString("Returns the point at the given t-value.");
  RaverieFullBindMethod(builder, type, &Segment::GetTValue, RaverieNoOverload, "GetTValue", "point")->Description = RaverieDocumentString("Returns the t-value that would result in the given "
                                                                                                                                          "point projected onto the segment.");
  type->ToStringFunction = Raverie::BoundTypeToGlobalToString<Segment>;
  type->AddAttribute(ExportDocumentation);
}

RaverieDefineExternalBaseType(Aabb, TypeCopyMode::ValueType, builder, type)
{
  RaverieBindDefaultCopyDestructor();
  RaverieFullBindConstructor(builder, type, RaverieSelf, "center, halfExtents", Vec3Param, Vec3Param);

  RaverieBindMemberProperty(mMin);
  RaverieBindMemberProperty(mMax);

  RaverieBindOverloadedMethodAs(Expand, RaverieStaticOverload(Aabb, const Aabb&, Vec3Param), "Expand")->Description = RaverieDocumentString("Creates an aabb that contains the given aabb and point.");
  RaverieBindOverloadedMethodAs(Combine, RaverieStaticOverload(Aabb, const Aabb&, const Aabb&), "Expand")->Description = RaverieDocumentString("Creates an aabb that contains the two given aabbs.");
  RaverieBindOverloadedMethodAs(Expand, RaverieInstanceOverload(void, Vec3Param), "Expand")->Description = RaverieDocumentString("Expand this aabb to contain the given point.");
  RaverieBindOverloadedMethodAs(Combine, RaverieInstanceOverload(void, const Aabb&), "Expand")->Description = RaverieDocumentString("Expand this aabb to contain the given aabb.");
  RaverieBindMethod(ContainsPoint)->Description = RaverieDocumentString("Does this aabb contain the given point?");
  RaverieBindMethodAs(Overlap, "Overlaps")->Description = RaverieDocumentString("Does this aabb overlap/intersect the given aabb?");
  RaverieFullBindMethod(builder, type, &Aabb::Set, RaverieNoOverload, "Set", "point");
  RaverieFullBindMethod(builder, type, &Aabb::SetCenterAndHalfExtents, RaverieNoOverload, "Set", "center, halfExtents");
  RaverieBindMethod(SetInvalid)->Description = RaverieDocumentString("Sets this aabb to an invalid aabb "
                                                                     "(Real3.PositiveMax, Real3.NegativeMin)). "
                                                                     "This also makes expansion easier.");

  RaverieBindGetterSetter(Extents);
  RaverieBindGetterSetter(HalfExtents);
  RaverieFullBindGetterSetter(builder, type, &Aabb::GetCenter, (Vec3(Aabb::*)() const), &Aabb::SetCenter, (void(Aabb::*)(Vec3Param)), "Center");

  // Expose later when we do a more full pass on the math library and geometry
  // RaverieFullBindMethod(builder, type, &Aabb::TransformAabb,
  // RaverieConstInstanceOverload(Aabb, Mat4Param), "Transform", "transform")
  //  ->Description = RaverieDocumentString("Computes the aabb of the current aabb
  //  after applying the given transformation.");

  RaverieBindGetter(Volume);
  RaverieBindGetter(SurfaceArea);

  RaverieBindMethodAs(Zero, "ZeroOut");
  type->ToStringFunction = Raverie::BoundTypeToGlobalToString<Aabb>;
  type->AddAttribute(ExportDocumentation);

  // Deprecated fns
  Raverie::Function* overlapFn = RaverieBindMethodAs(Overlap, "Overlap");
  overlapFn->Description = "This function is deprecated. Use Overlaps instead";
  overlapFn->AddAttribute(DeprecatedAttribute);
}

RaverieDefineExternalBaseType(Sphere, TypeCopyMode::ValueType, builder, type)
{
  RaverieBindDefaultCopyDestructor();
  RaverieFullBindConstructor(builder, type, RaverieSelf, "center, radius", Vec3Param, real);

  RaverieBindMemberProperty(mCenter);
  RaverieBindMemberProperty(mRadius);
  RaverieBindOverloadedMethodAs(Expand, RaverieStaticOverload(Sphere, const Sphere&, Vec3Param), "Expand")->Description =
      RaverieDocumentString("Creates a sphere that contains the given sphere and point.");
  RaverieBindOverloadedMethodAs(Expand, RaverieInstanceOverload(void, Vec3Param), "Expand")->Description = RaverieDocumentString("Expand this sphere to contain the given point.");
  RaverieBindMethodAs(Overlap, "Overlaps")->Description = RaverieDocumentString("Does this sphere overlap/intersect the given sphere?");
  RaverieBindGetter(Volume);
  RaverieBindGetter(SurfaceArea);
  type->ToStringFunction = Raverie::BoundTypeToGlobalToString<Sphere>;
  type->AddAttribute(ExportDocumentation);

  // Deprecated fns
  Raverie::Function* overlapFn = RaverieBindMethodAs(Overlap, "Overlap");
  overlapFn->Description = "This function is deprecated. Use Overlaps instead";
  overlapFn->AddAttribute(DeprecatedAttribute);
}

RaverieDefineExternalBaseType(Plane, TypeCopyMode::ValueType, builder, type)
{
  RaverieBindDefaultCopyDestructor();
  RaverieFullBindConstructor(builder, type, RaverieSelf, "data", Vec4Param);
  RaverieFullBindConstructor(builder, type, RaverieSelf, "normal point", Vec3Param, Vec3Param);
  RaverieFullBindMethod(builder, type, &RaverieSelf::Set, RaverieInstanceOverload(void, Vec4Param), "Set", "data");
  RaverieFullBindMethod(builder, type, &RaverieSelf::Set, RaverieInstanceOverload(void, Vec3Param, Vec3Param), "Set", "normal point");
  RaverieBindGetter(Normal);
  RaverieBindGetter(Distance);
  RaverieBindMemberProperty(mData);
  type->ToStringFunction = Raverie::BoundTypeToGlobalToString<Plane>;
  type->AddAttribute(ExportDocumentation);
}

RaverieDefineExternalBaseType(Frustum, TypeCopyMode::ValueType, builder, type)
{
  //@JoshD: Make the bound data actually useful (also fix ToString)
  RaverieBindDefaultCopyDestructor();
  RaverieBindMethod(Get);
  RaverieBindOverloadedMethod(Set, RaverieInstanceOverload(void, uint, const Plane&));
  RaverieBindMethod(GetAabb);
  type->ToStringFunction = Raverie::BoundTypeToGlobalToString<Frustum>;
  type->AddAttribute(ExportDocumentation);
}

RaverieDefineStaticLibrary(GeometryLibrary)
{
  RaverieInitializeExternalType(Ray);
  RaverieInitializeExternalType(Segment);
  RaverieInitializeExternalType(Aabb);
  RaverieInitializeExternalType(Sphere);
  RaverieInitializeExternalType(Plane);
  RaverieInitializeExternalType(Frustum);

#define RaverieDebugPrimitive(X) RaverieInitializeTypeAs(Debug::X, "Debug" #X);
#include "Foundation/Geometry/DebugPrimitives.inl"
#undef RaverieDebugPrimitive
}

void GeometryLibrary::Initialize()
{
  BuildStaticLibrary();
}

void GeometryLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Raverie
