// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class PropertyEditAction : public PropertyWidget
{
public:
  typedef PropertyEditAction RaverieSelf;

  PropertyEditAction(PropertyWidgetInitializer& init, Raverie::Function* method, HandleParam instance);

  void OnButtonPress(Event* event);

  Raverie::Function* mMethod;
  Handle mInstance;
  Element* mAction;
  TextButton* mButton;
};

class AddObjectWidget : public PropertyWidget
{
public:
  RaverieDeclareType(AddObjectWidget, TypeCopyMode::ReferenceType);
  AddObjectWidget(PropertyWidgetInitializer& init, PropertyWidgetObject* parentNode, PropertyView* grid, HandleParam instance);

  void UpdateTransform() override;

  // Open the add search window
  FloatingSearchView* OpenSearch(Vec3 position);

  void OnLeftClick(MouseEvent* event);
  void OnPostResourceAdded(PostAddResourceEvent* event);
  void OnAlternateSearchCompleted(AlternateSearchCompletedEvent* event);
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

} // namespace Raverie
