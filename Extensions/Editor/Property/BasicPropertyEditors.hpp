///////////////////////////////////////////////////////////////////////////////
///
/// \file BasicEditors.hpp
/// 
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//Forward Declarations
class Cog;
class Mouse;

void RegisterGeneralEditors();
void RegisterEngineEditors();

namespace Events
{
  DeclareEvent(ObjectPoll);
}

namespace PropertyViewUi
{
  DeclareTweakable(Vec4, ColorWidgetHighlight);
}

class ValueEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  Any NewValue;
  uint UserData;
};

class ObjectPollEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ObjectPollEvent() : FoundObject(NULL), OwnedWidget(NULL) {}
  Widget* OwnedWidget;
  Cog* FoundObject;
  Vec2 Position;
};

/// Direct property mix in (does common operations for property)
class DirectProperty : public PropertyWidget
{
public:
  typedef DirectProperty ZilchSelf;
  DirectProperty(PropertyWidgetInitializer& initializer);

  /// Returns whether or not the value is valid. For example, it could be
  /// invalid if this is a multi-selection and there is a conflict between
  /// the values on multiple objects
  PropertyState GetValue();

  void BeginPreviewChanges();
  void EndPreviewChanges();

  /// Set the editing value
  void PreviewValue(AnyParam value);
  void PreviewValue(PropertyState& state);

  /// Set the value and queue and undo 
  void CommitValue(AnyParam value);
  void CommitValue(PropertyState& state);

  void BuildPath(ObjectPropertyNode* node, Handle& rootInstance, PropertyPath& path);
  bool IsModified();

  /// Interface PropertyWidget
  String GetToolTip(ToolTipColor::Enum* color) override;

  void OnRightMouseUpLabel(MouseEvent* event);

  void Refresh() override;

  void OnRevert(Event* e);
  void OnMarkModified(Event* e);

  Property* mProperty;
  Handle mInstance;
  PropertyStateCapture mCapture;
  ObjectPropertyNode* mNode;
  bool mReadOnly;
};

namespace PropertyViewUi
{
  DeclareTweakable(Vec4, ModifiedTextColor);
}

}//namespace Zero
