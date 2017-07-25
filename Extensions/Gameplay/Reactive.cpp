///////////////////////////////////////////////////////////////////////////////
///
/// \file Reactive.cpp
/// Implementation of the Reactive component class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void Reactive::Initialize(CogInitializer& initializer)
{
  HasOrAdd<ReactiveSpace>(GetSpace());
}

void Reactive::Serialize(Serializer& stream)
{
  SerializeName(mActive);
}

ZilchDefineType(Reactive, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);
  ZeroBindDocumented();

  ZilchBindFieldProperty(mActive);

  ZeroBindEvent(Events::MouseEnter, MouseEvent);
  ZeroBindEvent(Events::MouseEnterPreview, MouseEvent);
  ZeroBindEvent(Events::MouseExit, MouseEvent);

  ZeroBindEvent(Events::MouseEnterHierarchy, MouseEvent);
  ZeroBindEvent(Events::MouseExitHierarchy, MouseEvent);

  ZeroBindEvent(Events::MouseMove, MouseEvent);
  ZeroBindEvent(Events::MouseUpdate, MouseEvent);
  ZeroBindEvent(Events::MouseScroll, MouseEvent);

  ZeroBindEvent(Events::DoubleClick, MouseEvent);

  ZeroBindEvent(Events::MouseDown, MouseEvent);
  ZeroBindEvent(Events::MouseUp, MouseEvent);

  ZeroBindEvent(Events::LeftMouseDown, MouseEvent);
  ZeroBindEvent(Events::LeftMouseUp, MouseEvent);

  ZeroBindEvent(Events::RightMouseDown, MouseEvent);
  ZeroBindEvent(Events::RightMouseUp, MouseEvent);

  ZeroBindEvent(Events::MiddleMouseDown, MouseEvent);
  ZeroBindEvent(Events::MiddleMouseUp, MouseEvent);

  ZeroBindEvent(Events::LeftClick, MouseEvent);
  ZeroBindEvent(Events::RightClick, MouseEvent);
  ZeroBindEvent(Events::MiddleClick, MouseEvent);

  ZeroBindEvent(Events::MouseHold, MouseEvent);
  ZeroBindEvent(Events::MouseHover, MouseEvent);
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


ZilchDefineType(ReactiveSpace, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZeroBindDependency(Space);

  ZilchBindGetterProperty(Over);

  // Set meta composition
  type->Add(new RaycasterMetaComposition(offsetof(ReactiveSpace, mRaycaster)));
}

ReactiveSpace::ReactiveSpace()
{
  mRaycaster.AddProvider(new PhysicsRaycastProvider());

  GraphicsRaycastProvider* graphicsRaycaster = new GraphicsRaycastProvider();
  mRaycaster.AddProvider(graphicsRaycaster);

  graphicsRaycaster->mVisibleOnly = true;
}

void ReactiveSpace::Initialize(CogInitializer& initializer)
{
}

void ReactiveSpace::Serialize(Serializer& stream)
{
}
Cog* ReactiveSpace::GetOver()
{
  return mOver;
}

} // namespace Zero
