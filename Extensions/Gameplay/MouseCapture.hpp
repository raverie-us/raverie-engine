///////////////////////////////////////////////////////////////////////////////
///
/// \file MouseCapture.hpp
/// Declaration of the MouseCapture Component.
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(MouseDragStart);
  DeclareEvent(MouseDragMove);
  DeclareEvent(MouseDragEnd);
}

class MouseEvent;
class ViewportMouseEvent;

//----------------------------------------------------------- Mouse Manipulation
class MouseCapture : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor / Destructor.
  MouseCapture(){}
  ~MouseCapture();

  /// Component Interface.
  void Serialize(Serializer& stream) override;

  /// Starts the mouse manipulation. All mouse events will now only go to
  /// the owner of this Component. Returns whether or not the manipulation
  /// can be started.
  bool Capture(ViewportMouseEvent* e);

  /// Releases the mouse manipulation.
  void ReleaseCapture();

  /// Returns whether or not this has an active mouse capture.
  bool IsCaptured();

  /// We need to release the mouse manipulation when we're destroyed.
  void OnDestroy(u32 flags = 0) override;

public:
  bool mPreventNextMouseUp;

private:
  /// The manipulation object.
  HandleOf<MouseManipulation> mManipulation;
};

} // namespace Zero
