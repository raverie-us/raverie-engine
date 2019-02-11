// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

static const real sViewCubeMinSize = 0.1f;
static const real sViewCubeMaxSize = 0.3f;

ZilchDefineType(EditorSettings, builder, type)
{
  type->AddAttribute(ObjectAttributes::cCore);

  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZilchBindFieldProperty(mViewCube);
  ZilchBindGetterSetterProperty(ViewCubeSize)
      ->Add(new EditorSlider(sViewCubeMinSize, sViewCubeMaxSize, 0.01f));
  ZilchBindFieldProperty(mAutoUpdateContentChanges);
}

EditorSettings::EditorSettings()
{
}

void EditorSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mViewCube, true);
  SerializeNameDefault(mViewCubeSize, 0.12f);
  SerializeNameDefault(mAutoUpdateContentChanges, true);
}

real EditorSettings::GetViewCubeSize()
{
  return mViewCubeSize;
}

void EditorSettings::SetViewCubeSize(real size)
{
  mViewCubeSize = Math::Clamp(size, sViewCubeMinSize, sViewCubeMaxSize);
}

} // namespace Zero
