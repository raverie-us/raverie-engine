///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Forward Declarations.
class TextButton;

//----------------------------------------------------------------------- Events
namespace Events
{
  // Sent when the modal was closed
  DeclareEvent(ModalClosed);
  DeclareEvent(ModalConfirmResult);
  DeclareEvent(ModalButtonPressed);
}

//--------------------------------------------------------- Modal Confirm Result
class ModalConfirmEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ModalConfirmEvent();

  /// Whether or not confirmed was pressed.
  bool mConfirmed;
  /// The user data from the modal (the modal could be closed)
  void* mUserData;
  String mStringUserData;
};

//--------------------------------------------------------- Modal Confirm Result
/// Event for the ModalButtonsAction class to send that a button with a given name was pressed.
class ModalButtonEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  String mButtonName;

  /// The user data from the modal (the modal could be closed)
  void* mUserData;
  String mStringUserData;
};

//------------------------------------------------------------------------ Modal
DeclareEnum2(ModalSizeMode, Fixed, Percentage);

class Modal : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  Modal(Composite* parent, float fadeInTime = 0.25f);

  /// Composite Interface.
  void UpdateTransform() override;

  /// The color of the background.
  void SetBackgroundColor(Vec4Param color);

  /// Fades out and closes the modal. Can be handled by any derived modal
  /// (which should then call CloseInternal). This is so ModalConfirm can send
  /// out a cancel when closed for any reason (such as loosing focus).
  virtual void Close(float fadeOutTime = 0.15f);

public:
  /// Close when the background is clicked on.
  void OnBackgroundClicked(Event*);

  /// Close when escape is pressed.
  void OnKeyDown(KeyboardEvent* e);

  bool mCloseOnBackgroundClicked;
  bool mCloseOnEscape;

  /// Dims and blocks input to everything behind the modal.
  Element* mBackground;

  /// Any custom data the user wants to attach to the modal to make callbacks easier.
  void* mUserData;
  String mStringUserData;
};

//------------------------------------------------------------------ Modal Strip
/// A Modal with a horizontal strip down the center. Attaching widgets
/// to this Modal will attach them to the strip.
class ModalStrip : public Modal
{
public:
  /// Constructor.
  ModalStrip(Composite* parent, float fadeInTime = 0.25f);

  /// Composite Interface.
  void UpdateTransform() override;

  /// Attach all child widgets to the strip Composite.
  void AttachChildWidget(Widget* widget, AttachType::Enum attachType) override;

  /// Sets the color of the strip highlight.
  void SetStripColor(Vec4Param color);

  /// The height of the strip.
  void SetStripHeight(ModalSizeMode::Type mode, float height);

  ModalSizeMode::Type mStripHeightMode;
  float mStripHeight;

  /// A strip that goes across the modal dialog. All content is displayed
  /// inside this strip (if it's displayed).
  Element* mStrip;

  /// The content centered in the Modal.
  Composite* mStripArea;
};

//--------------------------------------------------------- Modal Confirm Action
class ModalConfirmAction : public ModalStrip
{
public:
  /// Typedefs.
  typedef ModalConfirmAction ZilchSelf;

  /// Constructor.
  ModalConfirmAction(Composite* parent, StringParam title, float fadeInTime = 0.25f);

  void Close(float fadeOutTime = 0.15f) override;

  /// Button Event Response.
  void OnConfirmPressed(Event* e);
  void OnCancelPressed(Event* e);

  void Confirm();
  void Cancel();
  void SendEvent(bool confirmed);

  bool TakeFocusOverride() override;

  /// Whether or not to automatically close when a selection is made.
  bool mCloseOnSelection;

  Text* mTitle;
  TextButton* mConfirm;
  TextButton* mCancel;
};

//--------------------------------------------------------- ModalButtonsAction
// A modal that has an array of buttons
class ModalButtonsAction : public ModalStrip
{
public:
  /// Typedefs.
  typedef ModalButtonsAction ZilchSelf;

  /// Construct an with an array of buttons
  ModalButtonsAction(Composite* parent, StringParam title, Array<String>& buttonNames, StringParam extraText = String(), float fadeInTime = 0.25f);
  /// Construct with the given string as the only button (just a helper to make construction easier)
  ModalButtonsAction(Composite* parent, StringParam title, StringParam buttonName, StringParam extraText = String(), float fadeInTime = 0.25f);
  
  /// The actual helper that makes the buttons (do not call directly)
  void CreateButtons(StringParam title, Array<String>& buttonNames, StringParam extraText);

  void Close(float fadeOutTime = 0.15f) override;

  /// Button Event Response.
  void OnButtonPressed(ObjectEvent* e);

  void SendButtonPressed(TextButton* button);

  bool TakeFocusOverride() override;

  /// Whether or not to automatically close when a selection is made.
  bool mCloseOnSelection;

  Text* mTitle;
  Composite* mCenter;
  Array<TextButton*> mButtons;
};

}//namespace Zero
