///////////////////////////////////////////////////////////////////////////////
///
/// \file Reactive.hpp
/// Declaration of the Reactive component class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class MouseEvent;

DeclareBitField3(MouseCaptureButtons, Left, Right, Middle);

//--------------------------------------------------------------------- Reactive
/// Reactive component allows the object to react to mouse events.
/// Uses the collision volume of the collider on this composition for picking.
class Reactive : public Component
{
public:
  // Meta Initialization
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Reactive();
  ~Reactive();

  //Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void SetDefaults() override;

  //Events
  void OnLeftMouseDown(MouseEvent* event);
  void OnLeftMouseUp(MouseEvent* event);
  void OnMiddleMouseDown(MouseEvent* event);
  void OnMiddleMouseUp(MouseEvent* event);
  void OnRightMouseDown(MouseEvent* event);
  void OnRightMouseUp(MouseEvent* event);

  /// Is the reactive component active.
  bool mActive;
};

//--------------------------------------------------------------- Reactive Space
class ReactiveSpace : public Component
{
public:
  // Meta Initialization
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ReactiveSpace();

  //Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  Cog* GetOver();

  /// The object we're currently hovering over
  CogId mOver;

  // Lets us generically ray-cast into a scene (physics, graphics, etc)
  Raycaster mRaycaster;
};

} // namespace Zero
