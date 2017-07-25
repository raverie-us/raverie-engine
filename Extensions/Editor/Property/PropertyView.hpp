///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyView.hpp
/// Declaration of PropertyView and supporting classes.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class PropertySource;
class EditorPropertyExtension;
class PropertyView;
class PropertyWidgetObject;
class UpdateEvent;
class ContextMenu;

//----------------------------------------------------------------------- Events
namespace Events
{
  DeclareEvent(NameActivated);
  DeclareEvent(OpenAdd);
  DeclareEvent(PropertyContextMenu);
}//namespace Events

/// Used to allow external objects to add items to a context menu.
class ContextMenuEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ContextMenu* mMenu;
  Property* mProperty;
  Handle mInstance;
};

//---------------------------------------------------------------- Property Grid
typedef Widget* (*CustomIconCreatorFunction)(Composite* parent, HandleParam object,
                                             Property* metaProperty, void* clientData);

///Property Grid allow the editing of properties on objects.
class PropertyView : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PropertyView(Composite* parent);
  virtual ~PropertyView();

  /// Set the object to be edited. If no property interface is given,
  /// the current interface (default if none was ever set) will be used.
  /// If it's a multi selection, the instance should be the Selection object.
  void SetObject(HandleParam instance,  PropertyInterface* newInterface = nullptr);
  void SetObject(Object* object);

  /// The object being edited.
  Handle GetObject();

  /// Composite Interface.
  void UpdateTransform() override;
  void SizeToContents() override;
  Vec2 GetMinSize() override;

  /// Clears the Property
  void Invalidate();

  /// Refreshes all the values of all visible property widgets.
  void Refresh();

  /// Refreshes the tree every EngineUpdate.
  void ActivateAutoUpdate();

  /// Sets the interface used for getting / setting values on properties.
  void SetPropertyInterface(PropertyInterface* propertyInterface,
                            bool rebuild = false);

  /// Sets a callback for adding a custom icon to the left of the name
  /// of each property widget. This will rebuild the property grid. To remove
  /// the custom icon, pass in NULL for the callback.
  void AddCustomPropertyIcon(CustomIconCreatorFunction callback,
                             void* clientData = nullptr);
  void RemoveCustomPropertyIcon(CustomIconCreatorFunction callback);

  /// The percentage of width that the name takes up for each property widget.
  float mNamePercent;

  /// Will use the object's properties to determine minimum height
  bool mFixedHeight;

  // Rebuild all property widgets.
  void Rebuild();

protected:
  friend class PropertyWidget;
  friend class PropertyWidgetObject;

  /// Used for the auto update feature. Refreshes the tree every update.
  void OnWidgetUpdate(UpdateEvent* update);

  /// When the components of any selected objects changes, we need
  /// to rebuild the entire tree.
  void OnInvalidate(Event* e);

  void OnKeyDown(KeyboardEvent* e);

  /// The root of the visible widgets representing each node in the
  /// ObjectPropertyNode tree.
  PropertyWidgetObject* mRoot;

  /// The primary object we're displaying properties of. If it's a multi
  /// selection, this object should be the Selection object.
  Handle mSelectedObject;

  /// The main objects we have selected. This is for multi select.
  Array<Handle> mSelectedObjects;

  /// The default property interface used if none is specified.
  PropertyInterface mDefaultPropertyInterface;
  
  /// The current property interface. Could point to the default interface.
  PropertyInterface* mPropertyInterface;

  /// The scroll area that all property widgets are attached to
  ScrollArea* mScrollArea;

  /// Collection of additional widgets attached to this control
  Array<Widget*> mAddtionalWidgets;

public:
  /// A callback used for attaching a custom icon to the left of the properties.
  typedef Array<CustomIconCreatorFunction> CustomPropertyIconArray;
  CustomPropertyIconArray mCustomIconCallbacks;

  /// Client data given in the custom icon callback. The key is the callback 
  /// casted to a void*.
  HashMap<void*, void*> mCallbackClientData;
};

/// Tweakables
namespace PropertyViewUi
{
DeclareTweakable(float, ObjectSize);
DeclareTweakable(float, PropertySize);
DeclareTweakable(float, PropertySpacing);
DeclareTweakable(float, IndentSize);
}//namespace PropertyViewUi

}//namespace Zero
