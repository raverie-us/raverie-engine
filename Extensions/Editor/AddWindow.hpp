////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(AddWindowCancelled);
  DeclareEvent(ResourceTypeSelected);
  DeclareEvent(ResourceTemplateSelected);
}

// After we create a new Resource, it will be assigned to the object and property defined here.
struct PostAddOp
{
  Handle mObject;
  PropertyPath mProperty;
};

class ReourceManagersDataSource;
class AddResourceWindow;
class ResourceTypeSearch;
class ResourceTemplateSearch;
class ResourceTemplateDisplay;
class ResourceTemplateDataSource;
class ImportButton;

// Open an add window
AddResourceWindow* OpenAddWindow(BoundType* resourceType, Window** window = NULL);

//------------------------------------------------------------------------------ Add Resource Window
class AddResourceWindow : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AddResourceWindow(Composite* parent);

  /// Shows template's for the given resource type.
  void SelectResourceType(BoundType* resourceType);

  /// Used to hide the Resource type search on the left.
  void ShowResourceTypeSearch(bool state);

  PostAddOp mPostAdd;

private:
  /// Forward focus to the resource type search box.
  bool TakeFocusOverride() override;

  void OnResourceTypeSelected(Event*);
  void OnResourceTemplateSelected(Event*);

  /// Search on the left side.
  ResourceTypeSearch* mResourceTypeSearch;
  ResourceTemplateSearch* mResourceTemplateSearch;
  ResourceTemplateDisplay* mResourceTemplateDisplay;
};

//----------------------------------------------------------------------------- Resource Type Search
class ResourceTypeSearch : public ColoredComposite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ResourceTypeSearch(Composite* parent);

  BoundType* GetSelectedResourceType();
  void SetSelectedResourceType(BoundType* resourceType);

private:
  friend AddResourceWindow;

  /// Forward focus to the search box.
  bool TakeFocusOverride() override;

  void OnImportClicked(Event*);
  void OnFilesSelected(OsFileSelection* file);

  /// Select the most appropriate row as they type.
  void OnTextEntered(Event* e);
  void OnKeyDownSearch(KeyboardEvent* e);
  void OnKeyDown(KeyboardEvent* e);

  /// When they hit 'Enter' on a selected Resource type, we want to notify a type was selected.
  void OnEnter(Event*);

  void OnResourceTypeSelected(Event*);

  Widget* mNextFocus;
  BoundType* mSelectedType;
  TagChainTextBox* mSearchField;
  ItemList* mResourceList;
  Array<String> mValidResourceTypes;
};

//------------------------------------------------------------------------- Resource Template Search
class ResourceTemplateSearch : public ColoredComposite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ResourceTemplateSearch(Composite* parent);

  /// Shows all templates for the given resource type.
  void ShowTemplates(BoundType* resourceType);

  Resource* GetSelectedResourceTemplate();

  uint mTemplateCount;

private:
  friend AddResourceWindow;
  void UpdateTransform() override;

  void OnImportClicked(Event*);
  void OnFilesSelected(OsFileSelection* file);

  /// Forward focus to the search box.
  bool TakeFocusOverride() override;

  void OnTemplateSelected(Event*);
  void OnContentComponentsChanged(Event*);
  void OnKeyDown(KeyboardEvent* e);

  ImportButton* mImportButton;
  Element* mNoTemplateBackground;
  Widget* mNextFocus;
  Widget* mPreviousFocus;
  Text* mNothingSelectedText;
  ResourceManager* mManager;
  ItemList* mTemplateList;
};

//------------------------------------------------------------------------ Resource Template Display
class ResourceTemplateDisplay : public ColoredComposite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ResourceTemplateDisplay(Composite* parent, PostAddOp& postAdd);

  void ShowResourceTemplate(Resource* resourceTemplate);
  void HidePreview();

private:
  friend AddResourceWindow;

  /// Forward focus to the search box.
  bool TakeFocusOverride() override;

  void UpdateTransform() override;

  void BuildContentLibraryList();
  void CreateNameToolTip(StringParam message);
  void RemoveNameToolTip();
  void OnTextTypedName(Event*);
  void OnKeyDownNameField(KeyboardEvent* e);
  bool ValidateName(bool finalValidation);
  void OnCancel(Event*);
  void OnCreate(Event*);
  
  PostAddOp& mPostAdd;

  Widget* mPreviousFocus;
  Element* mDisabledCover;
  TextButton* mCancelButton;
  Label* mTemplateName;
  Composite* mPreviewParent;
  Widget* mPreviewWidget;
  Text* mPreviewText;
  // The selected resource
  HandleOf<Resource> mSelectedTemplate;
  MultiLineText* mDescription;
  TextBox* mNameField;
  TextBox* mTagsBox;
  StringComboBox* mLibrarySelect;
  HandleOf<ToolTip> mNameToolTip;
};

//------------------------------------------------------------------------ Resource Template Display
class ImportButton : public ColoredComposite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ImportButton(Composite* parent);

  void OnMouseEnter(Event*);
  void OnLeftMouseDown(Event*);
  void OnLeftMouseUp(Event*);
  void OnMouseExit(Event*);

  Text* mText;
  Element* mIcon;
};

} // namespace Zero
