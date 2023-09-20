// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

/// Provides the interface for casting through graphics space.
class GraphicsRaycastProvider : public RaycastProvider
{
public:
  RaverieDeclareType(GraphicsRaycastProvider, TypeCopyMode::ReferenceType);

  GraphicsRaycastProvider();
  // This function currently only returns 1 result (2 if you get a display
  // mount and graphical object). Should be fixed later!!!
  void RayCast(Ray& ray, CastInfo& castInfo, RaycastResultList& results) override;
  void FrustumCast(Frustum& frustum, CastInfo& castInfo, RaycastResultList& results) override;

  bool RayTest(Graphical* graphical, GraphicsRayCast& rayCast, CastInfo& castInfo);
  bool FrustumTest(Graphical* graphical, Frustum& frustum, CastInfo& castInfo);

  /// Whether graphical objects should be selected with frustum cast.
  bool mMultiSelectGraphical;

  /// Only select visible objects.
  bool mVisibleOnly;
};

} // namespace Raverie
