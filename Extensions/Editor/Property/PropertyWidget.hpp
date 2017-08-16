///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyWidget.hpp
/// Declaration of PropertyWidget.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class PropertyView;
class PropertyWidgetObject;

//-------------------------------------------------- Property Widget Initializer
struct PropertyWidgetInitializer
{
  /// The parent composite.
  Composite* Parent;
  /// A pointer back the property grid.
  PropertyView* Grid;
  /// The property this widget is editing (if it's a property node).
  Property* Property;
  /// The interface used to set/get properties.
  PropertyInterface* CurrentInterface;
  /// The object instance.
  Handle Instance;
  ObjectPropertyNode* ObjectNode;
};

//------------------------------------------------------------------------------
namespace StyleMode
{
  enum Enum
  {
    Regular,
    Node
  };
}//namespace StyleMode

//-------------------------------------------------------------- Property Widget
class PropertyWidget : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor / Destructor.
  PropertyWidget(PropertyWidgetInitializer& i, 
                 StyleMode::Enum style = StyleMode::Regular);
  ~PropertyWidget();

  /// Composite interface.
  void UpdateTransform();

  /// Called when the property grid refreshes.
  virtual void Refresh() {}
  virtual String GetToolTip(ToolTipColor::Enum* color){return String();}

  //Helper functions for layout
  LayoutResult GetNameLayout();
  LayoutResult GetContentLayout(LayoutResult& nameLayout);

  /// Event response.
  void OnMouseHover(MouseEvent* event);

  /// Whether or not it's an object widget.
  virtual bool IsObjectWidget(){return false;}

  /// Used for animations. It's the position that the object should 
  /// be at in the layout.
  Vec3 mDestination;

  /// Our parent widget will always be a PropertyWidgetObject.
  PropertyWidgetObject* Parent;
 
  Link<PropertyWidget> link;
  PropertyView* mGrid;
  PropertyInterface* mProp;

  /// When it's animating, we don't want to lay it out.
  bool mAnimating;

protected:
  Composite* mCustomIcons;
  Label* mLabel;
};

typedef PropertyWidget* (*MakePropertyWidget)(PropertyWidgetInitializer& initializer);

//--------------------------------------------------------- Meta Property Editor
// Creates a custom editor widget for modifying properties.
// This meta component is expected to be found in two places:
//   1. On a property type (e.g. float, String, Resource, etc...)
//   2. On a EditorPropertyExtension
//     - These have priority over the property type, because it's on a per property basis
class MetaPropertyEditor : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MetaPropertyEditor(MakePropertyWidget make)
  {
    Maker = make;
  }
  
  PropertyWidget* CreateWidget(PropertyWidgetInitializer& initializer)
  {
    return Maker(initializer);
  }

  MakePropertyWidget Maker;
};

template<typename propEditorType>
PropertyWidget* CreateProperty(PropertyWidgetInitializer& initializer)
{
  return new propEditorType(initializer);
}

}//namespace Zero
