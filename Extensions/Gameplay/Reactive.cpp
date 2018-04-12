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

ZeroBindEvent(Events::MouseFileDrop, MouseFileDropEvent);

  ZeroBindEvent(Events::MouseEnter, ViewportMouseEvent);
  ZeroBindEvent(Events::MouseEnterPreview, ViewportMouseEvent);
  ZeroBindEvent(Events::MouseExit, ViewportMouseEvent);

  ZeroBindEvent(Events::MouseEnterHierarchy, ViewportMouseEvent);
  ZeroBindEvent(Events::MouseExitHierarchy, ViewportMouseEvent);

  ZeroBindEvent(Events::MouseMove, ViewportMouseEvent);
  ZeroBindEvent(Events::MouseUpdate, ViewportMouseEvent);
  ZeroBindEvent(Events::MouseScroll, ViewportMouseEvent);

  ZeroBindEvent(Events::DoubleClick, ViewportMouseEvent);

  ZeroBindEvent(Events::MouseDown, ViewportMouseEvent);
  ZeroBindEvent(Events::MouseUp, ViewportMouseEvent);

  ZeroBindEvent(Events::LeftMouseDown, ViewportMouseEvent);
  ZeroBindEvent(Events::LeftMouseUp, ViewportMouseEvent);

  ZeroBindEvent(Events::RightMouseDown, ViewportMouseEvent);
  ZeroBindEvent(Events::RightMouseUp, ViewportMouseEvent);

  ZeroBindEvent(Events::MiddleMouseDown, ViewportMouseEvent);
  ZeroBindEvent(Events::MiddleMouseUp, ViewportMouseEvent);

  ZeroBindEvent(Events::LeftClick, ViewportMouseEvent);
  ZeroBindEvent(Events::RightClick, ViewportMouseEvent);
  ZeroBindEvent(Events::MiddleClick, ViewportMouseEvent);

  ZeroBindEvent(Events::MouseHold, ViewportMouseEvent);
  ZeroBindEvent(Events::MouseHover, ViewportMouseEvent);
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

  ZilchBindGetter(Over);
  ZilchBindFieldProperty(mRaycaster);
}

void ReactiveSpace::Serialize(Serializer& stream)
{
  bool success = Serialization::Policy<Raycaster>::Serialize(stream, "Raycaster", mRaycaster);
  if(success == false)
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

} // namespace Zero
