// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void Reactive::Initialize(CogInitializer& initializer)
{
  HasOrAdd<ReactiveSpace>(GetSpace());
}

void Reactive::Serialize(Serializer& stream)
{
  SerializeName(mActive);
}

RaverieDefineType(Reactive, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::CallSetDefaults);
  RaverieBindDocumented();

  RaverieBindFieldProperty(mActive);

  RaverieBindEvent(Events::MouseFileDrop, MouseFileDropEvent);

  RaverieBindEvent(Events::MouseEnter, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseEnterPreview, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseExit, ViewportMouseEvent);

  RaverieBindEvent(Events::MouseEnterHierarchy, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseExitHierarchy, ViewportMouseEvent);

  RaverieBindEvent(Events::MouseMove, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseUpdate, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseScroll, ViewportMouseEvent);

  RaverieBindEvent(Events::DoubleClick, ViewportMouseEvent);

  RaverieBindEvent(Events::MouseDown, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseUp, ViewportMouseEvent);

  RaverieBindEvent(Events::LeftMouseDown, ViewportMouseEvent);
  RaverieBindEvent(Events::LeftMouseUp, ViewportMouseEvent);

  RaverieBindEvent(Events::RightMouseDown, ViewportMouseEvent);
  RaverieBindEvent(Events::RightMouseUp, ViewportMouseEvent);

  RaverieBindEvent(Events::MiddleMouseDown, ViewportMouseEvent);
  RaverieBindEvent(Events::MiddleMouseUp, ViewportMouseEvent);

  RaverieBindEvent(Events::LeftClick, ViewportMouseEvent);
  RaverieBindEvent(Events::RightClick, ViewportMouseEvent);
  RaverieBindEvent(Events::MiddleClick, ViewportMouseEvent);

  RaverieBindEvent(Events::MouseHold, ViewportMouseEvent);
  RaverieBindEvent(Events::MouseHover, ViewportMouseEvent);
}

void Reactive::SetDefaults()
{
  mActive = true;
}

Reactive::Reactive()
{
}

Reactive::~Reactive()
{
}

RaverieDefineType(ReactiveSpace, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();

  RaverieBindDependency(Space);

  RaverieBindGetter(Over);
  RaverieBindFieldProperty(mRaycaster);
}

void ReactiveSpace::Serialize(Serializer& stream)
{
  bool success = Serialization::Policy<Raycaster>::Serialize(stream, "Raycaster", mRaycaster);
  if (success == false)
  {
    mRaycaster.AddProvider(new PhysicsRaycastProvider());

    GraphicsRaycastProvider* graphicsRaycaster = new GraphicsRaycastProvider();
    graphicsRaycaster->mVisibleOnly = true;
    mRaycaster.AddProvider(graphicsRaycaster);
  }
}

Cog* ReactiveSpace::GetOver()
{
  return mOver;
}

} // namespace Raverie
