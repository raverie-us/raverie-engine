// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

class ViewportInterface : public Component
{
public:
  RaverieDeclareType(ViewportInterface, TypeCopyMode::ReferenceType);
  virtual float GetAspectRatio() = 0;
  virtual Vec2 GetViewportSize() = 0;
  virtual void SendSortEvent(GraphicalSortEvent* event) = 0;
  virtual Cog* GetCameraCog() = 0;
};

} // namespace Raverie
