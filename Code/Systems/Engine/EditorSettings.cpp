// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

static const real sViewCubeMinSize = 0.1f;
static const real sViewCubeMaxSize = 0.3f;

RaverieDefineType(EditorSettings, builder, type)
{
  type->AddAttribute(ObjectAttributes::cCore);

  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
  RaverieBindFieldProperty(mViewCube);
  RaverieBindGetterSetterProperty(ViewCubeSize)->Add(new EditorSlider(sViewCubeMinSize, sViewCubeMaxSize, 0.01f));
  RaverieBindFieldProperty(mAutoUpdateContentChanges);
}

EditorSettings::EditorSettings()
{
}

void EditorSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mViewCube, true);
  SerializeNameDefault(mViewCubeSize, 0.1f);
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

} // namespace Raverie
