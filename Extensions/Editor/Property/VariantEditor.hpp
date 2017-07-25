///////////////////////////////////////////////////////////////////////////////
///
/// \file VariantEditor.hpp
/// Defines value editors used by the columns in the tree view.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(TextUpdated);
} // namespace Events

///Simple event for general signals.
class TextUpdatedEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TextUpdatedEvent(Object* source) : mSource(source), mChangeAccepted(false) {};
  
  Object* GetSource();
  
  Object* mSource;
  bool mChangeAccepted;
};

DeclareBitField1(InPlaceTextEditorFlags, EditOnDoubleClick);

class FormattedInPlaceText
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  FormattedInPlaceText()
  {
    mTextColor = Vec4(1);
    mAdditionalTextColor = Vec4(1);
  }

  String mText;
  Vec4 mTextColor;

  // Extra text placed after 'mText' (was added to show a Cogs Archetype).
  String mAdditionalText;
  Vec4 mAdditionalTextColor;

  Array<String> mCustomIcons;
};

//--------------------------------------------------------------- Variant Editor
class ValueEditor : public Composite
{
public:
  ValueEditor(Composite* parent, bool editable = false) 
    : Composite(parent), Editable(editable)
  {};
  
  virtual void Edit(){}
  virtual void SetVariant(AnyParam variant)=0;
  virtual void GetVariant(Any& variant)=0;

  /// The object to connect to for dragging the row (such as the name text).
  virtual void GetDragWidgets(Array<Widget*>& widgets) { }

  String Name;
  bool Editable;
};

//------------------------------------------------------------------ Text Editor
class InPlaceTextEditor : public ValueEditor
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  InPlaceTextEditor(Composite* parent, u32 flags);

  /// Widget Interface.
  void SizeToContents() override;
  void UpdateTransform() override;

  /// ValueEditor Interface.
  void Edit() override;
  void SetVariant(AnyParam variant) override;
  void GetDragWidgets(Array<Widget*>& widgets) override;
  void GetVariant(Any& variant) override;

  void GetEditTextVariant(Any& variant);
  void SizeAllText();

  /// Event response.
  void OnTextBoxChanged(ObjectEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnTextLostFocus(FocusEvent* event);
  void OnDoubleClick(Event* event);

  float GetIconsWidth();

  Label* mText;
  // Extra text placed after 'mText' (was added to show a Cogs Archetype).
  Label* mAdditionalText;
  TextBox* mEdit;
  Array<Element*> mCustomIcons;
};

//--------------------------------------------------------- Value Editor Factory
const String cDefaultValueEditor = "TextEditor";
const String cDefaultResourceEditor = "ResourceEditor";
const String cDefaultIconEditor = "IconEditor";
const String cDefaultBooleanEditor = "BooleanEditor";

class ValueEditorFactory : public ExplicitSingleton<ValueEditorFactory, Object>
{
public:
  /// Typedefs
  typedef ValueEditor* (*ValueEditorCreator)(Composite* parent, AnyParam data, u32 flags);

  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  ValueEditorFactory();

  /// Registers a creator based on the given type.
  void RegisterEditor(StringParam type, ValueEditorCreator creator);

  /// Gets an editor based on the given type.
  ValueEditor* GetEditor(StringParam type, Composite* parent, AnyParam data, u32 flags);

private:
  /// Stores all creators.
  HashMap<String, ValueEditorCreator> mRegisteredEditors;
};

}//namespace Zero
