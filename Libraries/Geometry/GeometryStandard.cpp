// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineExternalBaseType(Ray, TypeCopyMode::ValueType, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "start, direction", Vec3Param, Vec3Param);

  ZilchBindMemberProperty(Start);
  ZilchBindMemberProperty(Direction);
  ZilchFullBindMethod(builder, type, &Ray::GetPoint, ZilchNoOverload, "GetPoint", "tValue")->Description =
      ZilchDocumentString("Returns the point at the given t-value.");
  ZilchFullBindMethod(builder, type, &Ray::GetTValue, ZilchNoOverload, "GetTValue", "point")->Description =
      ZilchDocumentString("Returns the t-value that would result in the given "
                          "point projected onto the ray.");
  type->ToStringFunction = Zilch::BoundTypeToGlobalToString<Ray>;
  type->AddAttribute(ExportDocumentation);
}

ZilchDefineExternalBaseType(Segment, TypeCopyMode::ValueType, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "start, end", Vec3Param, Vec3Param);

  ZilchBindMemberProperty(Start);
  ZilchBindMemberProperty(End);
  ZilchFullBindMethod(builder, type, &Segment::GetPoint, ZilchNoOverload, "GetPoint", "tValue")->Description =
      ZilchDocumentString("Returns the point at the given t-value.");
  ZilchFullBindMethod(builder, type, &Segment::GetTValue, ZilchNoOverload, "GetTValue", "point")->Description =
      ZilchDocumentString("Returns the t-value that would result in the given "
                          "point projected onto the segment.");
  type->ToStringFunction = Zilch::BoundTypeToGlobalToString<Segment>;
  type->AddAttribute(ExportDocumentation);
}

ZilchDefineExternalBaseType(Aabb, TypeCopyMode::ValueType, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "center, halfExtents", Vec3Param, Vec3Param);

  ZilchBindMemberProperty(mMin);
  ZilchBindMemberProperty(mMax);

  ZilchBindOverloadedMethodAs(Expand, ZilchStaticOverload(Aabb, const Aabb&, Vec3Param), "Expand")->Description =
      ZilchDocumentString("Creates an aabb that contains the given aabb and point.");
  ZilchBindOverloadedMethodAs(Combine, ZilchStaticOverload(Aabb, const Aabb&, const Aabb&), "Expand")->Description =
      ZilchDocumentString("Creates an aabb that contains the two given aabbs.");
  ZilchBindOverloadedMethodAs(Expand, ZilchInstanceOverload(void, Vec3Param), "Expand")->Description =
      ZilchDocumentString("Expand this aabb to contain the given point.");
  ZilchBindOverloadedMethodAs(Combine, ZilchInstanceOverload(void, const Aabb&), "Expand")->Description =
      ZilchDocumentString("Expand this aabb to contain the given aabb.");
  ZilchBindMethod(ContainsPoint)->Description = ZilchDocumentString("Does this aabb contain the given point?");
  ZilchBindMethodAs(Overlap, "Overlaps")->Description =
      ZilchDocumentString("Does this aabb overlap/intersect the given aabb?");
  ZilchFullBindMethod(builder, type, &Aabb::Set, ZilchNoOverload, "Set", "point");
  ZilchFullBindMethod(builder, type, &Aabb::SetCenterAndHalfExtents, ZilchNoOverload, "Set", "center, halfExtents");
  ZilchBindMethod(SetInvalid)->Description = ZilchDocumentString("Sets this aabb to an invalid aabb "
                                                                 "(Real3.PositiveMax, Real3.NegativeMin)). "
                                                                 "This also makes expansion easier.");

  ZilchBindGetterSetter(Extents);
  ZilchBindGetterSetter(HalfExtents);
  ZilchFullBindGetterSetter(builder,
                            type,
                            &Aabb::GetCenter,
                            (Vec3(Aabb::*)() const),
                            &Aabb::SetCenter,
                            (void (Aabb::*)(Vec3Param)),
                            "Center");

  // Expose later when we do a more full pass on the math library and geometry
  // ZilchFullBindMethod(builder, type, &Aabb::TransformAabb,
  // ZilchConstInstanceOverload(Aabb, Mat4Param), "Transform", "transform")
  //  ->Description = ZilchDocumentString("Computes the aabb of the current aabb
  //  after applying the given transformation.");

  ZilchBindGetter(Volume);
  ZilchBindGetter(SurfaceArea);

  ZilchBindMethodAs(Zero, "ZeroOut");
  type->ToStringFunction = Zilch::BoundTypeToGlobalToString<Aabb>;
  type->AddAttribute(ExportDocumentation);

  // Deprecated fns
  Zilch::Function* overlapFn = ZilchBindMethodAs(Overlap, "Overlap");
  overlapFn->Description = "This function is deprecated. Use Overlaps instead";
  overlapFn->AddAttribute(DeprecatedAttribute);
}

ZilchDefineExternalBaseType(Sphere, TypeCopyMode::ValueType, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "center, radius", Vec3Param, real);

  ZilchBindMemberProperty(mCenter);
  ZilchBindMemberProperty(mRadius);
  ZilchBindOverloadedMethodAs(Expand, ZilchStaticOverload(Sphere, const Sphere&, Vec3Param), "Expand")->Description =
      ZilchDocumentString("Creates a sphere that contains the given sphere and point.");
  ZilchBindOverloadedMethodAs(Expand, ZilchInstanceOverload(void, Vec3Param), "Expand")->Description =
      ZilchDocumentString("Expand this sphere to contain the given point.");
  ZilchBindMethodAs(Overlap, "Overlaps")->Description =
      ZilchDocumentString("Does this sphere overlap/intersect the given sphere?");
  ZilchBindGetter(Volume);
  ZilchBindGetter(SurfaceArea);
  type->ToStringFunction = Zilch::BoundTypeToGlobalToString<Sphere>;
  type->AddAttribute(ExportDocumentation);

  // Deprecated fns
  Zilch::Function* overlapFn = ZilchBindMethodAs(Overlap, "Overlap");
  overlapFn->Description = "This function is deprecated. Use Overlaps instead";
  overlapFn->AddAttribute(DeprecatedAttribute);
}

ZilchDefineExternalBaseType(Plane, TypeCopyMode::ValueType, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "data", Vec4Param);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "normal point", Vec3Param, Vec3Param);
  ZilchFullBindMethod(builder, type, &ZilchSelf::Set, ZilchInstanceOverload(void, Vec4Param), "Set", "data");
  ZilchFullBindMethod(
      builder, type, &ZilchSelf::Set, ZilchInstanceOverload(void, Vec3Param, Vec3Param), "Set", "normal point");
  ZilchBindGetter(Normal);
  ZilchBindGetter(Distance);
  ZilchBindMemberProperty(mData);
  type->ToStringFunction = Zilch::BoundTypeToGlobalToString<Plane>;
  type->AddAttribute(ExportDocumentation);
}

ZilchDefineExternalBaseType(Frustum, TypeCopyMode::ValueType, builder, type)
{
  //@JoshD: Make the bound data actually useful (also fix ToString)
  ZilchBindDefaultCopyDestructor();
  ZilchBindMethod(Get);
  ZilchBindOverloadedMethod(Set, ZilchInstanceOverload(void, uint, const Plane&));
  ZilchBindMethod(GetAabb);
  type->ToStringFunction = Zilch::BoundTypeToGlobalToString<Frustum>;
  type->AddAttribute(ExportDocumentation);
}

ZilchDefineStaticLibrary(GeometryLibrary)
{
  ZilchInitializeExternalType(Ray);
  ZilchInitializeExternalType(Segment);
  ZilchInitializeExternalType(Aabb);
  ZilchInitializeExternalType(Sphere);
  ZilchInitializeExternalType(Plane);
  ZilchInitializeExternalType(Frustum);

#define ZeroDebugPrimitive(X) ZilchInitializeTypeAs(Debug::X, "Debug" #X);
#include "Geometry/DebugPrimitives.inl"
#undef ZeroDebugPrimitive
}

void GeometryLibrary::Initialize()
{
  BuildStaticLibrary();
}

void GeometryLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Zero
