///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyWidgets.hpp
/// Declaration of PropertyWidget.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------- Property Edit Action
class PropertyEditAction : public PropertyWidget
{
public:
  typedef PropertyEditAction ZilchSelf;

  PropertyEditAction(PropertyWidgetInitializer& init, Function* method, HandleParam instance);

  void OnButtonPress(Event* event);

  Function* mMethod;
  Handle mInstance;
  Element* mAction;
  TextButton* mButton;
};

//------------------------------------------------------------ Add Object Widget
class AddObjectWidget : public PropertyWidget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  AddObjectWidget(PropertyWidgetInitializer& init, 
                  PropertyWidgetObject* parentNode, 
                  PropertyView* grid, 
                  HandleParam instance);

  void UpdateTransform() override;

  // Open the add search window
  FloatingSearchView* OpenSearch(Vec3 position);

  void OnLeftClick(MouseEvent* event);
  void OnSearchCompleted(SearchViewEvent* event);
  void OnMetaModified(Event* event);
  void OnMouseEnter(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);
  void OnOpenAdd(Event* e);

  PropertyWidgetObject* mParentWidgetObject;
  bool mMouseOver;
  Element* mBackground;
  Element* mBorder;
  Element* mAddIcon;
  Handle mObject;
  HandleOf<FloatingSearchView> mActiveSearch;
  HandleOf<MetaComposition> mComposition;
  HandleOf<MetaArray> mMetaArray;
};

}//namespace Zero
