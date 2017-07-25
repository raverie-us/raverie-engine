///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyWidgetObject.hpp
/// Declaration of PropertyEditorObject.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
extern const String cPropArrowRight;
extern const String cPropArrowDown;

class Mouse;

//------------------------------------------------------------------------------
namespace NodeState
{
  enum Enum
  {
    Open,
    Closed
  };
}//namespace NodeState

//------------------------------------------------------- Property Editor Object
class PropertyWidgetObject : public PropertyWidget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PropertyWidgetObject(PropertyWidgetInitializer& initializer, PropertyWidgetObject* parentNode,
                       StringParam removedTypeName = "");

  ~PropertyWidgetObject();

  /// Composite Interface.
  void UpdateTransform();
  void LayoutChildren(bool animate = false);

  bool IsObjectWidget() override {return true;}

  bool ArePropertiesModified();

  /// Refreshes all the data on the object
  void RefreshLabel();
  void Refresh() override;

  /// Adds a child widget.
  void AddSubProperty(PropertyWidget* newChild);

  /// Removes all child widgets.
  void CloseNode();
  /// Animates the node closed, calls the CloseNode function when the
  /// animation is finished.
  void AnimateCloseNode();

  /// Opens the node and fills it out with widgets based on children.
  void OpenNode(bool animate);

  /// Removal of the component.
  void RemoveSelf();
  void AnimateRemoveSelf();

  /// Event response.
  void OnMouseEnterTitle(MouseEvent* e);
  void OnMouseExitTitle(MouseEvent* e);
  void OnMouseEnterX(MouseEvent* e);
  void OnMouseExitX(MouseEvent* e);
  void OnViewDoc(ObjectEvent* e);
  void OnViewOnlineDocs(ObjectEvent* e);
  void OnRemove(ObjectEvent* e);
  void OnRestore(ObjectEvent* e);
  void OnRightClick(MouseEvent* e);
  void OnLeftClickRemove(MouseEvent* e);
  void OnLeftClickTitle(MouseEvent* e);
  void OnMouseDragTitle(MouseEvent* e);
  void OnMetaModified(MetaTypeEvent* e);
  void OnEditScriptPressed(Event* e);

  void HighlightRed(StringParam message);
  void RemoveRedHighlight();
  void CreateTooltip(StringParam message, ToolTipColor::Enum color);

  /// Called at the end of actions to let the parent know that
  /// it can start laying out this widget.
  void AnimationFinished(){mAnimating = false;}

  Handle GetParentObject();

  void StartChildDrag(Mouse* mouse, PropertyWidgetObject* child);
  void EndDrag();

  uint GetComponentIndex();

  ObjectPropertyNode* mNode;

  HandleOf<MetaComposition> mComposition;
  HandleOf<MetaComposition> mParentComposition;

  HandleOf<MetaArray> mMetaArray;
  HandleOf<MetaArray> mParentMetaArray;

  PropertyWidgetObject* mParentWidgetObject;

  Element* mLocalModificationIcon;

  /// The background behind all properties. This is only visible when
  /// the property node is opened.
  Element* mBackground;

  /// The title bar.
  Element* mTitleBackground;

  /// The icon on the left of the title bar that depicts whether
  /// or not we're expanded.
  Element* mExpandNode;

  IconButton* mEditScriptButton;
  Element* mProxyIcon;

  /// When pressed, removes the component.
  Element* mRemoveIcon;
  HandleOf<ToolTip> mToolTip;
  bool mLocallyAdded;
  bool mLocallyRemoved;
  String mLocallyRemovedTypeName;

  /// Whether or not the mouse is currently over the title bar.
  bool mMouseOverTitle;
  bool mDragging;

  /// All children widgets owned by this object node.
  InList<PropertyWidget> ChildWidgets;

  Link<PropertyWidgetObject> mComponentLink;
  typedef InList<PropertyWidgetObject, &PropertyWidgetObject::mComponentLink> ComponentList;
  ComponentList mComponents;

  /// Custom ui attached to the bottom of this object.
  Composite* mCustomUi;

  /// Closed or open.
  NodeState::Enum mNodeState;
  MetaPropertyEditor* mData;

  /// Access back to the property grid that owns us.
  PropertyView* mGrid;

  static HashSet<String> mExpandedTypes;
};

namespace ComponentUi
{
DeclareTweakable(Vec4, TitleColor);
DeclareTweakable(Vec4, TitleHighlight);
DeclareTweakable(Vec4, TitleRemove);
DeclareTweakable(Vec4, BackgroundColor);
}

}//namespace Zero
