// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class ContentLibrary;

class EditorSettings : public Component
{
public:
  RaverieDeclareType(EditorSettings, TypeCopyMode::ReferenceType);

  /// Constructor.
  EditorSettings();

  /// Component Interface.
  void Serialize(Serializer& stream) override;

  real GetViewCubeSize();
  void SetViewCubeSize(real size);

  /// Show/hide the orientation view cube
  bool mViewCube;

  /// Size in pixels of the viewport the view cube is shown in
  real mViewCubeSize;
  bool mAutoUpdateContentChanges;
};

} // namespace Raverie
