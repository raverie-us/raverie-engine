///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations.
class ViewportMouseEvent;
class KeyboardEvent;
class OsWindow;

//------------------------------------------------------------ Root Input Router
/// UiRootInputRouter forwards input events to the Root Widget.
/// This can be done through two different methods:
/// 1. Operating System events.
///   - In the case that this is the top most UI under the OS window.
/// 2. Reactive Viewport events.
///   - In the case that this exists under the top most UI, such as in a game.
///     This way, the Ui can interact with other reactive objects.
class UiRootInputRouter : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  UiRootInputRouter();

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Reactive event response
  void OnMouseEvent(ViewportMouseEvent* e);
  void OnMouseButton(ViewportMouseEvent* e);
  void OnMouseUpdate(ViewportMouseEvent* e);
  void OnKeyboardEvent(KeyboardEvent* e);

  /// If set, all input from the Os will be forwarded to the root widget.
  void SetOsWindow(OsWindow* window);

  /// All input will be forwarded to the root widget.
  UiRootWidget* mRootWidget;

  /// When we forward events through the RootWidget, they will bubble back up
  /// and we'll get them again. This is used to ignore duplicate events.
  bool mIgnoreEvents;
};

}//namespace Zero
