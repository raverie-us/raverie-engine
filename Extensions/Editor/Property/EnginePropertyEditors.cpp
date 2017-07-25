///////////////////////////////////////////////////////////////////////////////
///
/// \file EnginePropertyEditors.cpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
// Because CogPath is a shared reference counted object, it doesn't actually matter if the user marks it as read only
// because it can still be modified by setting values on the path object itself (the actual cog path object can't be replaced with another though)
// However, if a reference property is read only, even if it is modified currently the property view will not mark the level as modified (issue)
static bool cCogPathIgnoreReadOnly = false;

namespace ComponentUi
{
DeclareTweakable(float, OpenTime);
}

namespace ResourceEditorUi
{
const cstr cLocation = "EditorUi/PropertyView/Editors";
Tweakable(Vec4, ResourceEditColor,     Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ResourceEditMouseOver, Vec4(1,1,1,1), cLocation);
Tweakable(float, CogPathPickerIndent, 8.0f, cLocation);
Tweakable(float, CogPathCogLabelMinWidth, 15.0f, cLocation);
Tweakable(float, CogPathCogLabelSpace, 100.0f, cLocation);
}

//------------------------------------------------------------ Archetype Property
class PropertyArchetype : public DirectProperty
{
public:
  typedef PropertyArchetype ZilchSelf;
  TextBox* mEditText;
  IconButton* mUpload;
  IconButton* mUploadInherit;
  IconButton* mRevert;
  IconButton* mDownload;
  Any mVariantValue;
  Composite* mRow;

  PropertyArchetype(PropertyWidgetInitializer& initializer)
    :DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();
    mLabel->SetText("Archetype");

    mRow = new Composite(this);
    mRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
    mEditText = new TextBox(mRow);
    mEditText->SetEditable(true);
    mEditText->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
    mEditText->mMeasureForMinSize = false;
    mEditText->SetMinSize(Pixels(20, 2));
    mRevert = new IconButton(mRow);
    mRevert->SetIcon("Revert");
    mUploadInherit = new IconButton(mRow);
    mUploadInherit->SetIcon("UploadInheritDisabled");
    mUpload = new IconButton(mRow);
    mUpload->SetIcon("Upload");

    if(mInstance.Get<MetaSelection*>())
      mEditText->SetReadOnly(true);

    Refresh();

    ConnectThisTo(mEditText, Events::TextEnter, OnTextEnter);

    ConnectThisTo(mUpload, Events::ButtonPressed, OnUpload);
    mUpload->SetToolTip("Upload to Archetype");

    ConnectThisTo(mRevert, Events::ButtonPressed, OnRevert);

    ConnectThisTo(mUploadInherit, Events::ButtonPressed, OnUploadInherited);

    ConnectThisTo(initializer.Grid, Events::MetaDrop, OnMetaDrop);
    ConnectThisTo(initializer.Grid, Events::MetaDropTest, OnMetaDrop);
  }

  Cog* GetCog()
  {
    if(MetaSelection* selection = mInstance.Get<MetaSelection*>())
      return selection->GetPrimaryAs<Cog>();
    
    return mInstance.Get<Cog*>();
  }

  void RevertCog(Cog* cog, HashSet<MetaSelection*>* modifiedSelections)
  {
    RevertToArchetype(Z::gEditor->GetOperationQueue(), cog);
  }

  void OnRevert(ObjectEvent* event)
  {
    // Revert all selected objects
    if(MetaSelection* selection = mInstance.Get<MetaSelection*>())
    {
      HashSet<MetaSelection*> modifiedSelections;

      OperationQueue* opQueue = Z::gEditor->GetOperationQueue();

      opQueue->BeginBatch();
      opQueue->SetActiveBatchName("RevertObjects");

      // Revert all selected objects. We don't want to rebuild archetypes
      // until we've marked each one as modified
      forRange(Cog* cog, selection->AllOfType<Cog>())
        RevertCog(cog, &modifiedSelections);

      opQueue->EndBatch();

      forRange(MetaSelection* selection, modifiedSelections.All())
        selection->FinalSelectionChanged();
    }
    else
    {
      // If it's just a single cog, revert it and rebuild archetypes
      Cog* cog = GetCog();
      if(cog->FindRootArchetype() != nullptr)
        RevertCog(cog, nullptr);
    }
  }

  void OnUploadInherited(ObjectEvent* e)
  {
    String archetypeName = mEditText->GetText();

    Cog* cog = GetCog();
    Archetype* oldArchetype = cog->GetArchetype();

    OperationQueue* opQueue = Z::gEditor->GetOperationQueue();
    UploadToArchetype(opQueue, cog, archetypeName, oldArchetype);

    // If the object had a locally removed Component, we need to rebuild the property grid
    mGrid->Invalidate();
  }

  void OnUpload(ObjectEvent* event)
  {
    String archetypeName = mEditText->GetText();

    OperationQueue* opQueue = Z::gEditor->GetOperationQueue();

    if(MetaSelection* selection = mInstance.Get<MetaSelection*>())
    {
      // Copy the objects from the selection as uploading to Archetype
      // will cause the selection to be modified
      Array<Cog*> cogs;
      forRange(Cog* cog, selection->AllOfType<Cog>())
      {
        if(cog->GetArchetype())
          cogs.PushBack(cog);
      }

      opQueue->BeginBatch();
      opQueue->SetActiveBatchName("UploadArchetypes");

      HashMap<Archetype*, uint> selectedArchetypes;
      forRange(Cog* cog, cogs.All())
      {
        Archetype* archetype = cog->GetArchetype();

        selectedArchetypes[archetype]++;
        uint count = selectedArchetypes[archetype];

        // If it's the first time we've seen the Archetype, upload this one
        if(count == 1)
        {
          // Uploading to current archetype
          UploadToArchetype(opQueue, cog);
        }
        // If it's the second, notify them that it was ignored
        // Ignore all other occurrences of this Archetype
        else if(count == 2)
        {
          cstr archetypeName = archetype->Name.c_str();
          String message = String::Format("There were multiple objects with "
            "the same Archetype (%s) in the selection. The first was "
            "uploaded, the others were ignored.", archetypeName);
          DoNotifyWarning("Object not Uploaded", message);
        }
      }
      opQueue->EndBatch();
    }
    else
    {
      Cog* cog = GetCog();
      Archetype* oldArchetype = cog->GetArchetype();

      // User may have just change archetype text and clicked upload
      // before pressing enter
      if(oldArchetype == nullptr || oldArchetype->Name != archetypeName)
      {
        UploadToArchetype(opQueue, cog, archetypeName);
        return;
      }

      // Uploading to current archetype
      UploadToArchetype(opQueue, cog);
    }

    // If the object had a locally removed Component, we need to rebuild the property grid
    mGrid->Invalidate();
  }

  void OnClear(MouseEvent* event)
  {
    OperationQueue* opQueue = Z::gEditor->GetOperationQueue();

    if(MetaSelection* selection = mInstance.Get<MetaSelection*>())
    {
      opQueue->BeginBatch();
      opQueue->SetActiveBatchName("PropertyEditors_OnSelectionClear");
      forRange(Cog* cog, selection->AllOfType<Cog>())
      {
        if(cog->GetArchetype())
          ClearArchetype(opQueue, cog);
      }
      opQueue->EndBatch();
    }
    else
    {
      Cog* cog = GetCog();
      if(cog->GetArchetype())
        ClearArchetype(opQueue, cog);
    }
  }

  void Refresh()
  {
    // If they are editing the archetype name or pressing
    // the archetype upload button, we don't want to set the archetype text
    bool subFocus = mEditText->HasFocus() || mUpload->HasFocus() || mUploadInherit->HasFocus();

    if(!subFocus)
    {
      PropertyState state = GetValue();
      if(state.IsValid())
      {
        mVariantValue = state.Value;
        if(Archetype* archetype = mVariantValue.Get<Archetype*>())
          mEditText->SetText(archetype->Name);
        else
          mEditText->SetText("");
      }
      else
      {
        mEditText->SetInvalid();
      }
  }

    LocalModifications* modifications = LocalModifications::GetInstance();

    HashSet<Archetype*> archetypes;
    // Whether or not there are modifications on any object
    bool revertableModifications = false;

    Archetype* singleArchetype = nullptr;
    Cog* singleCog = nullptr;
    Cog* commonArchetypeContextCog = nullptr;

    // Update button tooltips
    if(MetaSelection* selection = mInstance.Get<MetaSelection*>())
    {
      bool first = true;
      forRange(Cog* cog, selection->AllOfType<Cog>())
      {
        // Add all unique Archetypes
        if(Archetype* archetype = cog->GetArchetype())
          archetypes.Insert(archetype);

        if (first)
        {
          commonArchetypeContextCog = cog->FindNearestArchetypeContext();
          first = false;
        }
        else if(commonArchetypeContextCog)
        {
          // There is no common ancestor
          if (commonArchetypeContextCog != cog->FindNearestArchetypeContext())
            commonArchetypeContextCog = nullptr;
        }

        revertableModifications |= cog->IsModifiedFromArchetype();
      }
    }
    else
    {
      singleCog = GetCog();
      commonArchetypeContextCog = singleCog->FindNearestArchetypeContext();
      if (commonArchetypeContextCog == singleCog)
        commonArchetypeContextCog = nullptr;
      if(singleArchetype = singleCog->GetArchetype())
        archetypes.Insert(singleArchetype);

      if(singleCog->FindNearestArchetype())
        revertableModifications |= singleCog->IsModifiedFromArchetype();
    }

    // Upload inheritance button
    String currText = mEditText->GetText();
    if(singleArchetype == nullptr)
    {
      mUploadInherit->SetIcon("UploadInheritDisabled");
      mUploadInherit->SetToolTip("Requires a base Archetype to create an inherited Archetype");
      mUploadInherit->SetIgnoreInput(true);
    }
    else if(singleArchetype && subFocus && currText != singleArchetype->Name)
    {
      mUploadInherit->SetIcon("UploadInherit");

      String toolTip = String::Format("Uploaded Archetype inheriting from \"%s\"", singleArchetype->Name.c_str());
      mUploadInherit->SetToolTip(toolTip);
      mUploadInherit->SetIgnoreInput(false);
    }
    else
    {
      mUploadInherit->SetIcon("UploadInheritDisabled");
      mUploadInherit->SetToolTip("Enter new Archetype name to upload an inherited Archetype");
      mUploadInherit->SetIgnoreInput(true);
    }

    // Upload button
    if(archetypes.Empty() && mEditText->GetText().Empty())
    {
      mUpload->SetIcon("UploadDisabled");
      mUpload->SetIgnoreInput(true);
      mUpload->SetToolTip("Nothing to upload");
    }
    else
    {
      mUpload->SetIcon("Upload");
      mUpload->SetToolTip("Upload to Archetype");
      mUpload->SetIgnoreInput(false);
    }

    // Revert button
    if(revertableModifications)
    {
      mRevert->SetIcon("Revert");
      mRevert->SetIgnoreInput(false);
      
      String toolTip = "Revert object to Archetype";

      if(commonArchetypeContextCog)
      {
          toolTip = String::Format("Revert to the definition as specified by the parent '%s' Archetype",
                                   commonArchetypeContextCog->GetArchetype()->Name.c_str());
      }
      else if (archetypes.Empty())
      {
        toolTip = "Revert local changes";
      }

      mRevert->SetToolTip(toolTip);
    }
    else
    {
      mRevert->SetIcon("RevertDisabled");
      mRevert->SetIgnoreInput(true);
      mRevert->SetToolTip("Nothing to revert");
    }

    // Label text
    if(!archetypes.Empty() && revertableModifications)
      mLabel->SetColor(PropertyViewUi::ModifiedTextColor);
    else
      mLabel->SetColor(Vec4(1));
  }

  void OnTextEnter(ObjectEvent* event)
  {
    OperationQueue* opQueue = Z::gEditor->GetOperationQueue();

    String newValue = mEditText->GetText();
    if (newValue.Empty())
    {
      Cog* cog = GetCog();
      if(cog->GetArchetype())
        ClearArchetype(opQueue, cog);
    }
    else
    {
      // If string is not different then don't change anything
      Archetype* oldArchetype = GetCog()->GetArchetype();
      if (oldArchetype && oldArchetype->Name == newValue)
        return;

      //bool preExistingArchetype = (ArchetypeManager::GetInstance()->FindOrNull(newValue) != NULL);

      // MakeNewArchetypeWith will Assign the archetype to the cog
      Archetype* newArchetype = UploadToArchetype(opQueue, GetCog(), newValue);
      if(newArchetype == NULL)
        mEditText->SetText(String());

      // It is common to create a similarly purposed archetype by modifying a current one
      // If you don't want the tags copied over, you can clear the archetype field before making a new one
      //else if (oldArchetype && !preExistingArchetype)
      //{
      //  // Copy over tags
      //  TagList oldTags;
      //  oldArchetype->GetTags(oldTags);
      //  newArchetype->mContentItem->SetTags(oldTags);
      //}

      // Mark the translation as modified
      if(newArchetype)
      {
        Cog* cog = GetCog();
        cog->MarkTransformModified();
      }
    }
  }

  void UpdateTransform()
  {
    LayoutResult nameLayout = GetNameLayout();
    LayoutResult contentLayout = GetContentLayout(nameLayout);

    PlaceWithLayout(nameLayout, mLabel);
    PlaceWithLayout(contentLayout, mRow);

    mEditText->SetSize(Vec2(contentLayout.Size.x - Pixels(48), contentLayout.Size.y));

    PropertyWidget::UpdateTransform();
  }

  void OnMetaDrop(MetaDropEvent* event)
  {
    if(!event->Handled)
      DropOnObject(event, GetCog());
  }
};

//------------------------------------------------------------ Cog Reference
template <typename PropertyEditor>
class CogPickerManipulation : public MouseManipulation
{
public:

  typedef CogPickerManipulation ZilchSelf;

  CogPickerManipulation(Mouse* mouse, Composite* owner, PropertyEditor* editor)
    : MouseManipulation(mouse, owner)
  {
    mEditor = editor;
    mMousePosition = Vec2::cZero;
    mLastWidget = nullptr;
    mToolTip = nullptr;

    ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);
  }

  ~CogPickerManipulation()
  {
    mMouse->SetCursor(Cursor::Arrow);
    SafeDestroy(mToolTip);
  }

  CogId mSelectedObject;
  Widget* mLastWidget;

  ToolTip* mToolTip;
  PropertyEditor* mEditor;
  Vec2 mMousePosition;

  void OnUpdate(UpdateEvent* event);
  virtual void OnMouseMove(MouseEvent* event);
  virtual void OnMouseUp(MouseEvent* event);
  virtual void OnKeyDown(KeyboardEvent* event);
};

class PropertyEditorCogRef : public DirectProperty
{
public:
  typedef PropertyEditorCogRef ZilchSelf;
  TextBox* mName;
  PropertyState mState;

  Element* mPickerButton;
  Element* mClearButton;
  HandleOf<FloatingSearchView> mActiveSearch;

  PropertyEditorCogRef(PropertyWidgetInitializer& initializer)
    : DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();

    mName = new TextBox(this);

    mPickerButton = CreateAttached<Element>("Target");

    mClearButton = CreateAttached<Element>("WhiteX");
    mClearButton->SetColor(ToFloatColor(Color::OrangeRed));

    Refresh();

    ConnectThisTo(mPickerButton, Events::LeftMouseDown, OnPickerButtonDown);
    ConnectThisTo(mClearButton, Events::LeftClick, OnClearButtonClick);
    ConnectThisTo(mName, Events::LeftClick, OnLeftClick);
  }

  ~PropertyEditorCogRef()
  {
    mActiveSearch.SafeDestroy();
  }

  void ValidateSelection(Status& status, Cog* test)
  {
    // We should technically validate selections made from
    // inside an archetype to outside an archetype here
    // For now we return that all selections are valid
  }

  //Set the cog (Set null to clear)
  void SetReferencedCog(Cog* cog)
  {
    Any cogRef = cog;
    CommitValue(cogRef);

    // Refresh the property
    Refresh();
  }

  //Get the current Cog being referenced (may be null)
  Cog* GetReferencedCog()
  {
    // Get the CogId from the property
    mState = GetValue();
    if(!mState.IsValid())
      return NULL;

    return mState.Value.Get<Cog*>();
  }

  void Refresh() override
  {
    Cog* cog = GetReferencedCog();
    if(cog == nullptr)
      return;
    if(mState.IsValid())
    {
      String text = cog->GetDescription();
      mName->SetText(text);
    }
    else
    {
      mName->SetInvalid();
    }
  }

  void OnPickerButtonDown(MouseEvent* event)
  {
    if(mProperty->IsReadOnly())
      return;

    event->GetMouse()->SetCursor(Cursor::Cross);
    new CogPickerManipulation<PropertyEditorCogRef>(event->GetMouse(), GetParent(), this);
  }

  void OnClearButtonClick(MouseEvent* event)
  {
    if(mProperty->IsReadOnly())
      return;

    SetReferencedCog(NULL);
  }

  void OnLeftClick(MouseEvent* event)
  {
    if(mProperty->IsReadOnly())
      return;

    FloatingSearchView* searchView = mActiveSearch;
    if(searchView==NULL)
    {
      FloatingSearchView* viewPopUp = new FloatingSearchView(this);
      Vec3 mousePos = ToVector3(event->GetMouse()->GetClientPosition());
      SearchView* searchView = viewPopUp->mView;
      viewPopUp->SetSize(Pixels(300,400));
      viewPopUp->ShiftOntoScreen(mousePos);
      viewPopUp->UpdateTransformExternal();

      searchView->AddHiddenTag("Objects");
      searchView->mSearch->SearchProviders.PushBack(GetObjectSearchProvider()) ;

      searchView->TakeFocus();
      viewPopUp->UpdateTransformExternal();
      searchView->Search(String());
      ConnectThisTo(searchView, Events::SearchCompleted, OnSearchCompleted);

      mActiveSearch = viewPopUp;
    }
  }

  void OnSearchCompleted(SearchViewEvent* event)
  {
    Handle handle = event->Element->ObjectHandle;
    Cog* cog = handle.Get<Cog*>();
    SetReferencedCog(cog);
    mActiveSearch.SafeDestroy();
  }

  void UpdateTransform()
  {
    LayoutResult nameLayout = GetNameLayout();
    LayoutResult contentLayout = GetContentLayout(nameLayout);
    contentLayout.Size.x -= mPickerButton->GetSize().x + mClearButton->GetSize().x + Pixels(8);

    PlaceWithLayout(nameLayout, mLabel);
    PlaceWithLayout(contentLayout, mName);

    mClearButton->SetTranslation( Vec3(contentLayout.Translation.x + contentLayout.Size.x + Pixels(2),0,0) );
    mPickerButton->SetTranslation( Vec3(contentLayout.Translation.x + contentLayout.Size.x + Pixels(2) + mClearButton->GetSize().x + Pixels(2),0,0) );

    PropertyWidget::UpdateTransform();
  }

};

class PropertyEditorCogPath : public DirectProperty
{
public:
  typedef PropertyEditorCogPath ZilchSelf;

  Composite* mTopBar;
  Composite* mLabelPathIcons;
  Text* mCogText;
  TextBox* mPathTextBox;

  bool mExpanded;
  float mContractedSize;
  float mExpandedSize;
  IconButton* mPickerButton;
  IconButton* mExpandButton;
  HandleOf<FloatingSearchView> mActiveSearch;

  // A reference to the actual cog path
  CogPath mValue;

  PropertyWidgetObject* mMetaGeneratedProperties;
  ObjectPropertyNode* mObjectNode;

  PropertyEditorCogPath(PropertyWidgetInitializer& initializer)
    : DirectProperty(initializer)
  {
    mExpanded = false;
    mDefSet = initializer.Parent->GetDefinitionSet();
    SetName("CogPathEditor");
    Parent = Type::DynamicCast<PropertyWidgetObject*>(initializer.Parent);

    const float StackSpacing = 2.0f;
    SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2(0, StackSpacing), Thickness::cZero));

    mTopBar = new Composite(this);
    mTopBar->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness::cZero));

    mLabelPathIcons = new Composite(mTopBar);
    mLabelPathIcons->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
    mLabelPathIcons->AttachChildWidget(mLabel);
    new Spacer(mLabelPathIcons);

    Composite* cogTextParent = new Composite(mLabelPathIcons);
    mCogText = new Text(cogTextParent, DefaultTextStyle);
    mCogText->SetColor(Vec4(1.0f, 1.0f, 1.0f, 0.5f));
    mCogText->mClipText = true;

    mExpandButton = new IconButton(mLabelPathIcons);
    mExpandButton->SetIcon(cPropArrowRight);
    mExpandButton->mBackground->SetVisible(false);
    mExpandButton->mBorder->SetVisible(false);

    mPickerButton = new IconButton(mLabelPathIcons);
    mPickerButton->SetIcon("Target");
    mPickerButton->mBackground->SetVisible(false);
    mPickerButton->mBorder->SetVisible(false);

    mPathTextBox = new TextBox(mTopBar);
    mPathTextBox->SetEditable(true);
    mPathTextBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

    mMetaGeneratedProperties = nullptr;
    Refresh();

    mObjectNode = mProp->BuildObjectTree(nullptr, &mValue);

    mObjectNode->mProperty = initializer.ObjectNode->mProperty;
    mObjectNode->mParent = initializer.ObjectNode->mParent;

    PropertyWidgetInitializer initializerCogPath = initializer;
    initializerCogPath.ObjectNode = mObjectNode;
    initializerCogPath.Parent = this;
    mMetaGeneratedProperties = new PropertyWidgetObject(initializerCogPath, nullptr);
    mMetaGeneratedProperties->OpenNode(false);
    
    mTopBar->SizeToContents();
    
    mMetaGeneratedProperties->UpdateTransformExternal();
    float totalSizeY = 0.0f;
    int childCount = 0;
    forRange(Widget& child, GetChildren())
    {
      totalSizeY += child.mSize.y;
      ++childCount;
    }
    totalSizeY += StackSpacing * (childCount - 1);
    mSize.y = totalSizeY;
    
    mContractedSize = mTopBar->mSize.y;
    mExpandedSize = totalSizeY;
    mSize.y = mContractedSize;

    SetClipping(true);
    
    ConnectThisTo(mGrid, Events::PropertyModified, OnPropertyChanged);
    ConnectThisTo(mExpandButton, Events::LeftMouseDown, OnExpandButtonDown);
    ConnectThisTo(mPickerButton, Events::LeftMouseDown, OnPickerButtonDown);
    ConnectThisTo(mPickerButton, Events::RightMouseDown, OnPickerRightButtonDown);
    ConnectThisTo(mPathTextBox, Events::TextBoxChanged, OnTextEnter);
  }

  ~PropertyEditorCogPath()
  {
    mActiveSearch.SafeDestroy();
  }

  void ValidateSelection(Status& status, Cog* test)
  {
    CogPath::ComputePath(status, mValue.GetRelativeTo(), test, mValue.GetPathPreference0(), mValue.GetPathPreference1(), mValue.GetPathPreference2());
    if (status.Failed())
      return;

    if(AreTwoNamesTheSame(test))
    {
      status.SetFailed("Two objects have the same name");
      return;
    }
  }

  bool AreTwoNamesTheSame(Cog* test)
  {
    String name = test->GetName();

    Cog* parent = test->GetParent();
    if(parent)
    {
      size_t childCount = RangeCount(parent->FindAllChildrenByName(name));
      return childCount != 1;
    }
    else
    {
      // Check to see if there are multiple objects within the same space that have the same name
      Space* toSpace = test->GetSpace();
      if (toSpace != nullptr)
      {
        forRange(Cog& cog, toSpace->FindAllObjectsByName(name))
        {
          if (&cog != test && cog.GetName() == name)
            return true;
        }
      }
    }
    return false;
  }

  void SetReferencedCog(Cog* to)
  {
    if(to != nullptr)
    {
      if(AreTwoNamesTheSame(to))
        DoNotifyException("Cog Path", "Two objects have the same name (in the same space or under the same parent) so the CogPath may resolve to an incorrect object");

      //mValue.SetCog(to);

      //Variant result(mValue);
      //CommitValue(result);

      Handle rootInstance;
      PropertyPath propertyPath;
      BuildPath(mNode, rootInstance, propertyPath);

      Property* prop = ZilchTypeId(CogPath)->GetProperty("Cog");
      propertyPath.AddPropertyToPath(prop);

      PropertyState state(to);
      mProp->ChangeProperty(rootInstance, propertyPath, state, PropertyAction::Commit);

      // Refresh the property
      Refresh();
    }
  }
  
  void OnTextEnter(ObjectEvent* event)
  {
    Handle rootInstance;
    PropertyPath propertyPath;
    BuildPath(mNode, rootInstance, propertyPath);

    
    Property* prop = ZilchTypeId(CogPath)->GetProperty("Path");
    propertyPath.AddPropertyToPath(prop);

    PropertyState state(mPathTextBox->GetText());
    mProp->ChangeProperty(rootInstance, propertyPath, state, PropertyAction::Commit);

    //mValue.SetPath(mPathTextBox->GetText());
    //
    //Variant result(mValue);
    //CommitValue(result);
    //Refresh();
  }

  void Refresh() override
  {
    LocalModifications* modifications = LocalModifications::GetInstance();
    mLabel->SetColor(Vec4(1));
    if(ObjectState* objectState = modifications->GetObjectState(mInstance))
    {
      PropertyPath path(mProperty);

      forRange(Property* subProperty, ZilchTypeId(CogPath)->GetProperties())
      {
        path.AddPropertyToPath(subProperty);

        if(objectState->IsPropertyModified(path))
        {
          mLabel->SetColor(PropertyViewUi::ModifiedTextColor);
          break;
        }

        path.PopEntry();
      }
    }

    PropertyState state = GetValue();
    if(state.IsValid())
    {
      CogPath* path = state.Value.Get<CogPath*>();
      ReturnIf(!path,, "The value in the property grid was not a path (or it was not valid)");

      // Any changes to the actual CogPath in the variant we want to know about
      mValue = *path;

      Cog* cog = path->GetDirectCog();
      if (cog)
        mCogText->SetText(cog->GetDescription());
      else
        mCogText->SetText("");

      mExpandButton->SetVisible(true);
      mPickerButton->SetVisible(true);

      if(!cCogPathIgnoreReadOnly && mProperty->IsReadOnly())
      {
        mPathTextBox->SetReadOnly(true);
        mPickerButton->SetColor(Vec4(1.0f, 1.0f, 1.0f, 0.5f));
      }
      else
      {
        mPathTextBox->SetReadOnly(false);
        mPickerButton->SetColor(Vec4(1.0f));
      }

      if(!mPathTextBox->HasFocus())
        mPathTextBox->SetText(path->GetPath());
      
      if(mMetaGeneratedProperties != nullptr)
        mMetaGeneratedProperties->Refresh();
    }
    else
    {
      mPathTextBox->SetReadOnly(true);
      mPathTextBox->SetInvalid();
      mCogText->SetText("-");
      Expand(false, false);
      mExpandButton->SetVisible(false);
      mPickerButton->SetVisible(false);
    }
    if(mMetaGeneratedProperties)
      mMetaGeneratedProperties->Refresh();
  }

  void RefreshIfSameObject(HandleParam instance)
  {
    if(instance.Get<CogPath*>() == &mValue)
    {
      Any result(mValue);
      CommitValue(result);
      Refresh();
    }
  }

  void OnPropertyChanged(PropertyEvent* event)
  {
    // If the property that is being changed belongs to our temporary cog path object...
    RefreshIfSameObject(Handle(event->mObject));
  }

  void Expand(bool anmate, bool expanded)
  {
    mExpanded = expanded;

    float sizeY;
    if (expanded)
    {
      sizeY = mExpandedSize;
      mExpandButton->SetIcon(cPropArrowDown);
    }
    else
    {
      sizeY = mContractedSize;
      mExpandButton->SetIcon(cPropArrowRight);
    }

    if (anmate)
    {
      ActionSequence* sequence = new ActionSequence(this);
      sequence->Add(SizeWidgetAction(this, Vec2(mSize.x, sizeY), ComponentUi::OpenTime));
    }
    else
    {
      mSize.y = sizeY;
    }
  }

  void OnExpandButtonDown(MouseEvent* event)
  {
    Expand(true, !mExpanded);
  }

  void OnPickerButtonDown(MouseEvent* event)
  {
    if(!cCogPathIgnoreReadOnly && mProperty->IsReadOnly())
      return;

    event->GetMouse()->SetCursor(Cursor::Cross);
    new CogPickerManipulation<PropertyEditorCogPath>(event->GetMouse(), GetParent(), this);
  }

  void OnPickerRightButtonDown(MouseEvent* event)
  {
    if(!cCogPathIgnoreReadOnly && mProperty->IsReadOnly())
      return;

    FloatingSearchView* searchView = mActiveSearch;
    if(searchView==NULL)
    {
      FloatingSearchView* viewPopUp = new FloatingSearchView(this);
      Vec3 mousePos = ToVector3(event->GetMouse()->GetClientPosition());
      SearchView* searchView = viewPopUp->mView;
      viewPopUp->SetSize(Pixels(300,400));
      viewPopUp->ShiftOntoScreen(mousePos);
      viewPopUp->UpdateTransformExternal();

      searchView->AddHiddenTag("Objects");
      searchView->mSearch->SearchProviders.PushBack(GetObjectSearchProvider()) ;

      searchView->TakeFocus();
      viewPopUp->UpdateTransformExternal();
      searchView->Search(String());
      ConnectThisTo(searchView, Events::SearchCompleted, OnSearchCompleted);

      mActiveSearch = viewPopUp;
    }
  }

  void OnSearchCompleted(SearchViewEvent* event)
  {
    Cog* cog = (Cog*)event->Element->ObjectHandle.Get<Cog*>();
    SetReferencedCog(cog);
    mActiveSearch.SafeDestroy();
  }

  void UpdateTransform()
  {
    mTopBar->mSize.x = mSize.x;
    
    Composite* cogTextParent = mCogText->mParent;

    float sizeX = mSize.x - mLabel->mSize.x - mPickerButton->mSize.x - mExpandButton->mSize.x - ResourceEditorUi::CogPathCogLabelSpace;
    sizeX = Math::Max(sizeX, (float)ResourceEditorUi::CogPathCogLabelMinWidth);
    sizeX = Math::Min(sizeX, mCogText->GetMinSize().x);
    Vec2 cogTextSize = Vec2(sizeX, cogTextParent->mSize.y);
    cogTextParent->SetSize(cogTextSize);
    cogTextParent->SetMinSize(cogTextSize);
    mCogText->SetSize(cogTextSize);

    mMetaGeneratedProperties->mSize.x = mSize.x - ResourceEditorUi::CogPathPickerIndent;
    mMetaGeneratedProperties->mTranslation.x = ResourceEditorUi::CogPathPickerIndent;
    mMetaGeneratedProperties->UpdateTransformExternal();

    PropertyWidget::UpdateTransform();
  }

};

template <typename PropertyEditor>
void CogPickerManipulation<PropertyEditor>::OnUpdate(UpdateEvent* event)
{
  Cog* selectedObject = mSelectedObject;

  if(selectedObject)
  {
    // The size we want to display the text at
    const float TextSize = 0.55f;

    // We want to draw debug text in the object's space
    //Debug::DefaultConfig config;
    u32 spaceId = selectedObject->GetSpace()->GetId().Id;
    //config.SpaceId(spaceId);
    //config.OnTop(true);
    Debug::ActiveDrawSpace drawSpace(spaceId);

    String selectText = BuildString("Select '", selectedObject->GetDescription(), "'");

    Vec3 worldPosition = GetObjectTextPosition(selectedObject);

    SafeDestroy(mToolTip);

    ToolTipPlacement toolTipPlacement;
    toolTipPlacement.SetScreenRect(Rect::CenterAndSize(mMousePosition, Vec2(15, 15)));
    toolTipPlacement.SetPriority(IndicatorSide::Top, IndicatorSide::Bottom, IndicatorSide::Left, IndicatorSide::Right);
    mToolTip = new ToolTip(GetRootWidget());

    Status status;
    mEditor->ValidateSelection(status, selectedObject);
    if (status.Failed())
    {
      selectText = BuildString(selectText, "\n", status.Message);
      mToolTip->SetColor(ToolTipColor::Red);
    }
    
    mToolTip->SetText(selectText);
    mToolTip->SetArrowTipTranslation(toolTipPlacement);

    // Debug draw the object we're hovering over
    selectedObject->DebugDraw();
  }
}

template <typename PropertyEditor>
void CogPickerManipulation<PropertyEditor>::OnMouseMove(MouseEvent* event)
{
  mMousePosition = event->Position;

  ObjectPollEvent pollEvent;
  pollEvent.Position = event->Position;

  DispatchAtParams dispatchAtParams;
  dispatchAtParams.EventId = Events::ObjectPoll;
  dispatchAtParams.EventObject = &pollEvent;
  dispatchAtParams.Position = event->Position;

  this->GetRootWidget()->DispatchAt(dispatchAtParams);

  if(mLastWidget && mLastWidget != pollEvent.OwnedWidget)
    mLastWidget->GetDispatcher()->Dispatch(Events::MouseExit, event);

  if(pollEvent.OwnedWidget)
    pollEvent.OwnedWidget->GetDispatcher()->Dispatch(Events::MouseMove, event);

  mLastWidget = pollEvent.OwnedWidget;
  mSelectedObject = pollEvent.FoundObject;

  if(pollEvent.FoundObject == nullptr)
    SafeDestroy(mToolTip);
}

template <typename PropertyEditor>
void CogPickerManipulation<PropertyEditor>::OnMouseUp(MouseEvent* event)
{
  OnMouseMove(event);

  if (mSelectedObject)
  {
    mEditor->SetReferencedCog(mSelectedObject);
  }

  this->Destroy();
}

template <typename PropertyEditor>
void CogPickerManipulation<PropertyEditor>::OnKeyDown(KeyboardEvent* event)
{
  // If the user hit the escape key, destroy the mouse manipulator
  if (event->Key == Keys::Escape)
  {
    this->Destroy();
  }
}

class FocusComposite : public ColoredComposite
{
public:
  typedef FocusComposite ZilchSelf;

  FocusComposite(Composite* parent, Vec4Param color)
    : ColoredComposite(parent, color)
  {
    ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  }
  
  void OnKeyDown(KeyboardEvent* event)
  {
    TabJump(this, event);
  }

  bool TakeFocusOverride() override
  {
    this->HardTakeFocus();
    return true;
  }
};

class ResourceEditor : public Composite
{
public:
  typedef ResourceEditor ZilchSelf;

  BoundType* mResourceType;
  ResourceManager* mResourceManager;
  ColoredComposite* mResourcePreviewParent;
  Composite* mResourcePreview;
  ColoredComposite* mNameArea;
  Text* mResourceName;
  Element* mDownArrow;
  IconButton* mEditButton;
  IconButton* mClearButton;
  IconButton* mNewButton;
  IconButton* mCloneButton;
  Element* mBackground;
  Element* mBorder;
  Element* mFocusBorder;
  HandleOf<Resource> mDisplayedResource;
  bool mShowClearButton;
  bool mReadOnly;
  bool mSmallDisplay;

  ResourceEditor(Composite* parent, BoundType* resourceType)
    : Composite(parent)
    , mResourceType(resourceType)
    , mResourcePreview(nullptr)
    , mReadOnly(false)
  {
    // Background
    mBackground = CreateAttached<Element>(cWhiteSquare);
    mBackground->SetColor(Vec4(0.18f, 0.18f, 0.18f, 1.0f));
    mFocusBorder = CreateAttached<Element>(cWhiteSquareBorder);
    mFocusBorder->SetColor(Vec4(0.196f, 0.528f, 0.918f, 1));

    mBorder = CreateAttached<Element>(cWhiteSquareBorder);
    mBorder->SetColor(Vec4::cZero);

    SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(1, 0), Thickness(1, 1, 1, 1)));

    mResourceManager = Z::gResources->Managers.FindValue(mResourceType->Name, nullptr);

    mSmallDisplay = (ResourcePreview::GetPreviewImportance(resourceType) != PreviewImportance::High);

    if (mSmallDisplay)
      mSize.y = Pixels(16.0f);
    else
      mSize.y = Pixels(48.0f);

    // Remove button
    mClearButton = new IconButton(this);
    mClearButton->SetNotInLayout(true);
    mClearButton->SetIcon("RemoveX");
    mClearButton->mBackgroundColor = ToByteColor(Vec4(0.18f, 0.18f, 0.18f, 1.0f));
    mClearButton->mBackgroundHoverColor = ToByteColor(Vec4(0.43f, 0.18f, 0.18f, 1.0f));
    mClearButton->mPadding = Thickness(-1, 0, 0, 0);
    mClearButton->mBorder->SetActive(false);
    ConnectThisTo(mClearButton, Events::MouseEnterHierarchy, OnMouseEnterClear);
    ConnectThisTo(mClearButton, Events::MouseExitHierarchy, OnMouseExitClear);
    ConnectThisTo(mClearButton, Events::ButtonPressed, OnMouseExitClear);
    mClearButton->SetToolTip("Clear Resource");
    mClearButton->mTabFocusStop = false;

    Vec4 insetBackgroundColor = Vec4(0.4f, 0.4f, 0.4f, 1.0f);
    Vec4 insetHighlightColor = Vec4(0.45f, 0.45f, 0.45f, 1.0f);

    // Resource preview
    mResourcePreviewParent = new FocusComposite(this, insetBackgroundColor);
    if (mSmallDisplay)
      mResourcePreviewParent->SetSizing(SizePolicy::Fixed, Pixels(14, 14));
    else
      mResourcePreviewParent->SetSizing(SizePolicy::Fixed, Pixels(46, 46));
    mResourcePreviewParent->SetLayout(CreateFillLayout());
    mResourcePreviewParent->SetName("Resource Preview Parent");

    if (mSmallDisplay)
    {
      mResourcePreviewParent->SetNotInLayout(true);
      mResourcePreviewParent->mBackground->SetVisible(false);
    }

    // Resource name
    mNameArea = new ColoredComposite(this, Vec4(0.24f, 0.24f, 0.24f, 1.0f));
    mNameArea->SetSizing(SizePolicy::Flex, 1.0f);
    mNameArea->SetName("Name Area");
    {
      // Center resource text
      mResourceName = new Text(mNameArea, cText);
      mDownArrow = mNameArea->CreateAttached<Element>("DownArrow");
    }

    if (mSmallDisplay)
      mResourcePreviewParent->MoveToFront();

    if (resourceType->IsA(ZilchTypeId(ColorGradient)))
    {
      Composite* topDown = new Composite(this);
      topDown->SetSizing(SizePolicy::Flex, 1.0f);
      topDown->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 1)));
      topDown->AttachChildWidget(mNameArea);
      topDown->AttachChildWidget(mResourcePreviewParent);
      mNameArea->SetSize(Vec2(1));
      mNameArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(22));
      mResourcePreviewParent->SetSize(Vec2(1));
      mResourcePreviewParent->SetSizing(SizePolicy::Flex, 1.0f);
    }

    Composite* buttonParent = this;

    if (mSmallDisplay == false)
    {
      // Button Column
      buttonParent = new Composite(this);
      buttonParent->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(16));
      buttonParent->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 1)));
    }
    {
      mNewButton = new IconButton(buttonParent);
      mNewButton->mBackgroundColor = ToByteColor(insetBackgroundColor);
      mNewButton->mBackgroundHoverColor = ToByteColor(insetHighlightColor);
      mNewButton->mFocusBorderColor = ByteColorRGBA(50, 135, 234, 255);
      mNewButton->SetSizing(SizePolicy::Flex, 1);
      mNewButton->SetIcon("NewResource");
      mNewButton->SetToolTip("Create New Resource");
      mNewButton->mPadding = Thickness::cZero;
      mNewButton->mBorder->SetActive(false);

      mCloneButton = new IconButton(buttonParent);
      mCloneButton->mBackgroundColor = ToByteColor(insetBackgroundColor);
      mCloneButton->mBackgroundHoverColor = ToByteColor(insetHighlightColor);
      mCloneButton->mFocusBorderColor = ByteColorRGBA(50, 135, 234, 255);
      mCloneButton->SetSizing(SizePolicy::Flex, 1);
      mCloneButton->SetIcon("CloneResource");
      mCloneButton->SetToolTip("Clone Resource");
      mCloneButton->mPadding = Thickness::cZero;
      mCloneButton->mBorder->SetActive(false);
    }

    // Edit button
    mEditButton = new IconButton(this);
    mEditButton->mBackgroundColor = ToByteColor(insetBackgroundColor);
    mEditButton->mBackgroundHoverColor = ToByteColor(insetHighlightColor);
    mEditButton->mFocusBorderColor = ByteColorRGBA(50, 135, 234, 255);
    mEditButton->SetIcon("ResourceSelect");
    mEditButton->SetToolTip("Edit Resource");
    mEditButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(16));
    mEditButton->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
    mEditButton->mPadding = Thickness::cZero;
    mEditButton->mBorder->SetActive(false);

    if (mSmallDisplay)
    {
      mNewButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(24));
      mCloneButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(24));
      mEditButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(24));
    }

    if(mResourceManager->mCanDuplicate == false)
    {
      mCloneButton->mIconColor = ToByteColor(Vec4(1, 1, 1, 0.3f));
      mCloneButton->SetIgnoreInput(true);
      mCloneButton->mTabFocusStop = false;
      mCloneButton->SetToolTip("Cannot clone this Resource type");
      mCloneButton->mToolTipColor = ToolTipColor::Yellow;
    }

    if(mResourceManager->mCanCreateNew == false)
    {
      mNewButton->mIconColor = ToByteColor(Vec4(1, 1, 1, 0.3f));
      mNewButton->SetIgnoreInput(true);
      mNewButton->mTabFocusStop = false;
      mNewButton->SetToolTip(String::Format("Cannot create new %s", mResourceType->Name.c_str()));
      mNewButton->mToolTipColor = ToolTipColor::Yellow;
    }

    ConnectThisTo(mNameArea, Events::MouseEnterHierarchy, OnMouseEnterMainArea);
    ConnectThisTo(mResourcePreviewParent, Events::MouseEnterHierarchy, OnMouseEnterMainArea);
    ConnectThisTo(mNameArea, Events::MouseExitHierarchy, OnMouseExitMainArea);
    ConnectThisTo(mResourcePreviewParent, Events::MouseExitHierarchy, OnMouseExitMainArea);
    ConnectThisTo(mNameArea, Events::LeftMouseDown, OnLeftMouseDownMainArea);
    ConnectThisTo(mResourcePreviewParent, Events::LeftMouseDown, OnLeftMouseDownMainArea);
    ConnectThisTo(mNameArea, Events::LeftMouseUp, OnLeftMouseUpMainArea);
    ConnectThisTo(mResourcePreviewParent, Events::LeftMouseUp, OnLeftMouseUpMainArea);

    ConnectThisTo(mResourcePreviewParent, Events::FocusGained, OnFocusChanged);
    ConnectThisTo(mResourcePreviewParent, Events::FocusLost, OnFocusChanged);
  }

  void OnFocusChanged(Event*)
  {
    MarkAsNeedsUpdate();
  }

  void OnMouseEnterMainArea(Event*)
  {
    if(mReadOnly)
      return;
    mNameArea->mBackground->SetColor(Vec4(0.27f, 0.27f, 0.27f, 1.0f));
  }

  void OnMouseExitMainArea(Event*)
  {
    if(mReadOnly)
      return;
    mNameArea->mBackground->SetColor(Vec4(0.24f, 0.24f, 0.24f, 1.0f));
  }

  void OnLeftMouseDownMainArea(Event*)
  {
    if(mReadOnly)
      return;
    mNameArea->mBackground->SetColor(Vec4(0.23f, 0.23f, 0.23f, 1.0f));
  }

  void OnLeftMouseUpMainArea(Event*)
  {
    if(mReadOnly)
      return;
    mNameArea->mBackground->SetColor(Vec4(0.27f, 0.27f, 0.27f, 1.0f));
  }

  void OnMouseEnterClear(Event*)
  {
    if(mReadOnly)
      return;
    mBorder->SetColor(Vec4(0.58f, 0.18f, 0.18f, 1.0f / 1.4) * 1.4);
    mNameArea->mBackground->SetColor(Vec4(0.43f, 0.18f, 0.18f, 1.0f));
  }

  void OnMouseExitClear(Event*)
  {
    if(mReadOnly)
      return;
    mBorder->SetColor(Vec4::cZero);
    mNameArea->mBackground->SetColor(Vec4(0.24f, 0.24f, 0.24f, 1.0f));
  }

  void UpdateTransform() override
  {
    if(mSmallDisplay)
    {
      if (mClearButton->GetActive())
        mResourcePreviewParent->SetTranslation(Pixels(7, 1, 0));
      else
        mResourcePreviewParent->SetTranslation(Pixels(2, 1, 0));
    }

    Vec2 nameAreaSize = mNameArea->GetSize();
    Vec2 nameMinSize = mResourceName->GetMinSize();
    
    // Place the down arrow
    Vec2 arrowSize = mDownArrow->GetSize();
    Vec3 arrowTranslation(nameAreaSize.x - arrowSize.x - 12.0f, nameAreaSize.y * 0.5f - arrowSize.y * 0.5f, 0);
    mDownArrow->SetTranslation(SnapToPixels(arrowTranslation));

    // Place name
    Vec3 nameTranslation(12.0f, nameAreaSize.y * 0.5f - nameMinSize.y * 0.5f, 0);
    if (mSmallDisplay)
    {
      nameTranslation.x += Pixels(6);
      if (mClearButton->GetActive())
        nameTranslation.x += 5.0f;
    }
    mResourceName->SetTranslation(SnapToPixels(nameTranslation));

    Vec2 nameSize = nameAreaSize;
    nameSize.x -= 12.0f; // 12 pixels to the left of the name
    nameSize.x -= (nameAreaSize.x - arrowTranslation.x); // Space used up by the arrow
    nameSize.x -= 2.0f; // little breathing room between arrow and text
    mResourceName->SetSize(nameSize);

    // Clear placement
    if(mSmallDisplay)
      mClearButton->SetTranslation(Pixels(-6, 2, 0));
    else
      mClearButton->SetTranslation(Pixels(-6, 19, 0));
    mClearButton->SetSize(Pixels(11, 11));
    mClearButton->MoveToFront();

    mBackground->SetSize(mSize);
    mBorder->SetSize(mSize);

    Vec2 focusSize = mSize;
    focusSize.x = mNameArea->GetRectInParent().BottomRight().x + 1.0f;
    mFocusBorder->SetSize(focusSize);
    mFocusBorder->SetVisible(mResourcePreviewParent->HasFocus());
    Composite::UpdateTransform();
  }

  void SetResource(Resource* resource)
  {
    // Do nothing if we already have this resource displayed
    Resource* oldResource = mDisplayedResource;
    if (oldResource && oldResource == resource)
      return;

    if(oldResource)
      DisconnectAll(oldResource, this);

    mDisplayedResource = resource;

    // Destroy the old preview
    if (mResourcePreview && !(oldResource == nullptr && resource == nullptr))
    {
      mResourcePreview->Destroy();
      mResourceName->SetSizing(SizeAxis::Y, SizePolicy::Fixed, mResourceName->GetMinSize().y);
      mResourcePreview = nullptr;
    }

    mResourceName->mFontColor = Vec4(1);
    if (resource == nullptr)
    {
      mResourceName->SetText("nothing selected...");
      mResourceName->SetSizing(SizeAxis::Y, SizePolicy::Fixed, mResourceName->GetMinSize().y);
      mResourceName->mFontColor = Vec4(1, 1, 1, 0.4f);
      mClearButton->SetActive(false);
      mCloneButton->SetIgnoreInput(true);
      mCloneButton->mTabFocusStop = false;
      mCloneButton->mIconColor = ToByteColor(Vec4(1, 1, 1, 0.3f));
      mEditButton->SetIgnoreInput(true);
      mEditButton->mTabFocusStop = false;
      mEditButton->mIconColor = ToByteColor(Vec4(1, 1, 1, 0.3f));

      mResourcePreviewParent->SetActive(false);

      if (mResourceType->IsA(ZilchTypeId(ColorGradient)))
        mNameArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
      return;
    }

    mResourcePreviewParent->SetActive(true);

    if (mResourceType->IsA(ZilchTypeId(ColorGradient)))
      mNameArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(22));

    if(mResourceManager->mCanDuplicate)
    {
      mCloneButton->mIconColor = ToByteColor(Vec4(1));
      mCloneButton->mTabFocusStop = true;
      mCloneButton->SetIgnoreInput(false);
    }
    mEditButton->SetIgnoreInput(false);
    mEditButton->mIconColor = ToByteColor(Vec4(1));
    mEditButton->mTabFocusStop = true;

    ConnectThisTo(resource, Events::ObjectModified, OnResourceModified);

    if (mShowClearButton && !mReadOnly)
      mClearButton->SetActive(true);

    mResourceName->SetText(resource->Name);
    mResourceName->SetSizing(SizeAxis::Y, SizePolicy::Fixed, mResourceName->GetMinSize().y);

    mResourcePreview = ResourcePreview::CreatePreviewWidget(mResourcePreviewParent, resource->Name, resource);
    mResourcePreview->SetSizing(SizePolicy::Flex, 1.0f);
    if(mSmallDisplay)
      mResourcePreview->SetMinSize(Vec2(12, 12));
    else
      mResourcePreview->SetMinSize(Vec2(46, 46));

    if (mResourceType->IsA(ZilchTypeId(ColorGradient)))
      mResourcePreview->SetMinSize(Vec2(46, 5));
  }

  void OnResourceModified(Event*)
  {
    Resource* resource = mDisplayedResource;

    // Refresh
    SetResource(nullptr);
    SetResource(resource);
  }

  void SetReadOnly()
  {
    mReadOnly = true;

    mClearButton->SetActive(false);
    mDownArrow->SetActive(false);

    Vec4 alphaColor(1, 1, 1, 0.25f);
    mResourceName->SetColor(Vec4(1, 1, 1, 1.8f));
    mNameArea->SetColor(alphaColor);
    mResourcePreviewParent->SetColor(alphaColor);
    mNewButton->SetColor(alphaColor);
    mCloneButton->SetColor(alphaColor);

    mResourcePreviewParent->SetInteractive(false);
    mNewButton->SetInteractive(false);
    mCloneButton->SetInteractive(false);
  }

  void SetInvalid()
  {
    SetResource(nullptr);
    if (mShowClearButton)
      mClearButton->SetActive(true);

    mResourceName->SetText("-");
    mResourceName->SetSizing(SizeAxis::Y, SizePolicy::Fixed, mResourceName->GetMinSize().y);

    mEditButton->SetIgnoreInput(true);
    mEditButton->mIconColor = ToByteColor(Vec4(1, 1, 1, 0.3f));
  }

  void ShowClearButton(bool state)
  {
    mShowClearButton = state;
    mClearButton->SetActive(state);
  }
};

//------------------------------------------------------------ PropertyEditorResource
const String NoResourceName = "None";

class PropertyEditorResource : public DirectProperty
{
public:
  typedef PropertyEditorResource ZilchSelf;
  ComboBox* mSelectBox;
  Any mVariantValue;
  EditorResource* mMetaEdit;
  ResourceManager* mResourceManager;
  ResourceEditor* mEditor;
  // Storing a pointer to this is safe because when meta is changed, the entire property grid
  // is torn down and rebuilt. This should never point at a destroyed type.
  BoundType* mResourceType;
  HandleOf<FloatingSearchView> mActiveSearch;
  HandleOf<ToolTip> mTooltip;

  PropertyEditorResource(PropertyWidgetInitializer& initializer)
    : DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();
    mDefSet = mDefSet->GetDefinitionSet("PropertyGrid");

    mMetaEdit = mProperty->HasInherited<EditorResource>();

    mResourceType = Type::GetBoundType(initializer.Property->PropertyType);
    mResourceManager = Z::gResources->Managers.FindValue(mResourceType->Name, nullptr);

    ConnectThisTo(mResourceManager, Events::ResourceAdded, OnResourceAdded);
    
    mEditor = new ResourceEditor(this, mResourceType);

    SetSize(Pixels(1, mEditor->mSize.y + Pixels(20)));

    ConnectThisTo(mEditor->mResourcePreviewParent, Events::LeftClick, OnLeftClick);
    ConnectThisTo(mEditor->mNameArea, Events::LeftClick, OnLeftClick);

    ConnectThisTo(this, Events::MouseHover, OnHover);
    ConnectThisTo(this, Events::MouseExit, OnExit);

    ConnectThisTo(this, Events::MetaDrop, OnMetaDrop);
    ConnectThisTo(this, Events::MetaDropTest, OnMetaDrop);

    mEditor->ShowClearButton(false);
    if(mMetaEdit)
      mEditor->ShowClearButton(mMetaEdit->AllowNone);

    if (mProperty->IsReadOnly())
      mEditor->SetReadOnly();

    ConnectThisTo(mEditor->mEditButton, Events::ButtonPressed, OnEdit);
    ConnectThisTo(mEditor->mNewButton, Events::ButtonPressed, OnAdd);
    ConnectThisTo(mEditor->mClearButton, Events::ButtonPressed, OnRemove);
    ConnectThisTo(mEditor->mCloneButton, Events::ButtonPressed, OnDuplicate);
    ConnectThisTo(mEditor->mNameArea, Events::MouseEnterHierarchy, OnMouseEnter);
    ConnectThisTo(mEditor->mResourcePreviewParent, Events::MouseEnterHierarchy, OnMouseEnter);
    ConnectThisTo(mEditor->mNameArea, Events::MouseExitHierarchy, OnMouseExit);
    ConnectThisTo(mEditor->mResourcePreviewParent, Events::MouseExitHierarchy, OnMouseExit);

    ConnectThisTo(mEditor->mResourcePreviewParent, Events::KeyDown, OnFocusKeyDown);

    Refresh();
    this->SetName("ResourceSelector");
  }

  ~PropertyEditorResource()
  {
    if(FloatingSearchView* searchView = mActiveSearch)
      searchView->Destroy();
    mTooltip.SafeDestroy();
  }
  
  void OnFocusKeyDown(KeyboardEvent* e)
  {
    if (e->Key == Keys::Enter)
     OnLeftClick(nullptr);
    else if (e->Key == Keys::Delete || e->Key == Keys::Back)
    {
      // Only clear if it can be cleared
      if(mEditor->mClearButton->GetActive())
        OnRemove(nullptr);
    }
  }

  void OnMouseEnter(MouseEvent* e)
  {
    Pair<Resource*, PropertyState::Enum> state = GetResource();
    Resource* resource = state.first;
    if(resource == NULL || state.second != PropertyState::Valid)
      return;

    // Create the tooltip
    ToolTip* toolTip = new ToolTip(this);
    toolTip->SetDestroyOnMouseExit(false);
    toolTip->mContentPadding = Thickness(2, 2, 2, 2);
    toolTip->SetColor(ToolTipColor::Gray);

    // Create the resource widget and attach it to the tooltip
    String name = resource->Name;
    //MetaObjectInstance resourceInstance = resource;
    PreviewWidget* tileWidget = ResourcePreview::CreatePreviewWidget(toolTip, name, resource, PreviewImportance::High);
    if(tileWidget == NULL)
    {
      toolTip->Destroy();
      return;
    }
    toolTip->SetContent(tileWidget);

    // Position the tooltip
    Rect rect = this->GetScreenRect();

    // Offset out to look nicer
    float extraBorder = Pixels(15);
    rect.SizeX += extraBorder;
    rect.X -= extraBorder * 0.5f;

    ToolTipPlacement placement;
    placement.SetScreenRect(rect);
    placement.mHotSpot = mEditor->GetScreenRect().Center();
    placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, 
                          IndicatorSide::Bottom, IndicatorSide::Top);
    toolTip->SetArrowTipTranslation(placement);

    tileWidget->AnimatePreview(PreviewAnimate::Always);

    mTooltip = toolTip;
  }

  void OnMouseExit(MouseEvent* e)
  {
    mTooltip.SafeDestroy();
  }

  Pair<Resource*,PropertyState::Enum> GetResource()
  {
    PropertyState state = GetValue();
    if(!state.IsValid())
      return MakePair((Resource*)NULL, PropertyState::Invalid);
    mVariantValue = state.Value;

    if(mVariantValue.Is<String>())
    {
      Resource* resource = mResourceManager->GetResource(mVariantValue.ToString(), ResourceNotFound::ReturnNull);
      return MakePair(resource, PropertyState::Valid);
    }
    else
    {
      Resource* resource = mVariantValue.Get<Resource*>();
      return MakePair(resource, PropertyState::Valid);
    }
  }

  void SetResource(Resource* resource)
  {
    if(resource)
    {
      if(mProperty->PropertyType == ZilchTypeId(String))
      {
        // Set the as a string with the full Id and name
        mVariantValue = resource->ResourceIdName;
        CommitValue(mVariantValue);
      }
      else
      {
        // Set as handle type
        mVariantValue = resource;
        CommitValue(mVariantValue);
      }

      // Resource properties can invalid objects forcing the grid to rebuild
      if(mProperty->HasAttribute(PropertyAttributes::cInvalidatesObject))
        mGrid->Invalidate();
      else
        Refresh();
    }
  }

  void Refresh() override
  {
    DirectProperty::Refresh();

    auto resourceState = GetResource();
    if(resourceState.second == PropertyState::Valid)
      mEditor->SetResource(resourceState.first);
    else
      mEditor->SetInvalid();
  }

  void OnExit(MouseEvent* event)
  {

  }

  void OnHover(MouseEvent* event)
  {

  }

  void OnMetaDrop(MetaDropEvent* event)
  {
    if(event->Instance.StoredType == mResourceType)
    {
      event->Handled = true;
      Resource* resource = event->Instance.Get<Resource*>();
      if(event->Testing)
      {
        event->Result = String::Format("Set %s to %s", 
          mProperty->Name.c_str(), resource->Name.c_str());
      }
      else
      {
        mVariantValue = resource;
        CommitValue(mVariantValue);
      }
      Refresh();
    }
  }

  void OnRemove(ObjectEvent* event)
  {
    mEditor->SetResource(nullptr);

    if(mProperty->PropertyType == ZilchTypeId(String))
    {
      // Set the as a string with the full Id and name
      mVariantValue = String();
      CommitValue(mVariantValue);
    }
    else
    {
      // Set as handle type
      mVariantValue = Any(mResourceType);
      CommitValue(mVariantValue);
    }
  }

  void OnLeftClick(Event* event)
  {
    if(mProperty->IsReadOnly())
      return;
    FloatingSearchView* searchView = mActiveSearch;
    if(searchView == NULL)
    {
      FloatingSearchView* viewPopUp = new FloatingSearchView(this);
      SearchView* searchView = viewPopUp->mView;
      viewPopUp->SetSize(Pixels(300,400));
      Vec3 pos = ToVector3(mEditor->GetScreenRect().TopRight());
      viewPopUp->ShiftOntoScreen(pos + Vec3(6, 0, 0));
      viewPopUp->UpdateTransformExternal();

      searchView->AddHiddenTag("Resources");
      searchView->AddHiddenTag(mResourceManager->GetResourceType()->Name);

      searchView->AddMetaType(Type::GetBoundType(mProperty->PropertyType));

      if(mMetaEdit && !mMetaEdit->FilterTag.Empty())
        searchView->AddTag(mMetaEdit->FilterTag);

      searchView->mSearch->SearchProviders.PushBack(GetResourceSearchProvider());

      searchView->TakeFocus();
      viewPopUp->UpdateTransformExternal();
      searchView->Search(String());
      ConnectThisTo(searchView, Events::SearchCompleted, OnSearchCompleted);

      mActiveSearch = viewPopUp;
    }
  }

  void OnSearchCompleted(SearchViewEvent* event)
  {
    SetResource((Resource*)event->Element->Data);
    mActiveSearch.SafeDestroy();
  }
  
  void ResourcesChanged()
  {

  }

  void OnResourceAdded(ResourceEvent* event)
  {
    ResourcesChanged();
  }

  void OnAdd(Event* event)
  {
    mTooltip.SafeDestroy();
    Window* window = NULL;
    AddResourceWindow* addWidget = OpenAddWindow(mResourceType, &window);
    addWidget->ShowResourceTypeSearch(false);

    Rect rect = mEditor->GetScreenRect();
    Vec3 topRight = ToVector3(rect.TopRight());

    window->SetTranslation(topRight + Vec3(6.0f, -22.0f, 0));

    addWidget->mPostAdd.mObject = mInstance;
    addWidget->mPostAdd.mProperty = PropertyPath(mProperty->Name);
  }

  void OnDuplicate(Event* event)
  {
    mTooltip.SafeDestroy();

    if(Resource* resource = GetResource().first)
    {
      Resource* newResource = DuplicateResource(resource);
      SetResource(newResource);
    }
  }
  
  void OnEdit(Event* event)
  {
    Resource* resource = GetResource().first;
    if(resource)
    {
      Z::gEditor->EditResource(resource);
    }
  }

  void UpdateTransform()
  {
    LayoutResult nameLayout = GetNameLayout();
    PlaceWithLayout(nameLayout, mLabel);

    mEditor->SetTranslation(Vec3(6, 20.0f, 0));
    mEditor->SetSize(Vec2(mSize.x - 6, mEditor->GetSize().y));

    PropertyWidget::UpdateTransform();
  }

};

class ListItem : public Composite
{
public:
  typedef ListItem ZilchSelf;

  Element* mBackground;
  Element* mExpandIcon;
  Element* mRemoveIcon;
  Label* mLabel;

  ListItem(Composite* parent, StringParam itemName, bool bold, bool removable = false, bool expandable = false)
    : Composite(parent)
  {
    mBackground = CreateAttached<Element>(cWhiteSquare);
    mBackground->MoveToBack();
    mBackground->SetColor(ComponentUi::TitleColor);

    mLabel = new Label(this, bold ? "BoldText" : DefaultTextStyle);
    mLabel->SetInteractive(false);
    mLabel->SetClipping(true);
    mLabel->SetText(itemName);

    mExpandIcon = CreateAttached<Element>("PropArrowRight");
    mExpandIcon->SetVisible(expandable);
    mExpandIcon->SetInteractive(false);

    mRemoveIcon = CreateAttached<Element>("RemoveX");
    mRemoveIcon->SetVisible(removable);
    mRemoveIcon->SetInteractive(removable);

    ConnectThisTo(mBackground, Events::MouseEnter, OnMouseEnterBackground);
    ConnectThisTo(mBackground, Events::MouseExit, OnMouseExitBackground);

    ConnectThisTo(mRemoveIcon, Events::MouseEnter, OnMouseEnterRemove);
    ConnectThisTo(mRemoveIcon, Events::MouseExit, OnMouseExitRemove);

    SetSizing(SizeAxis::Y, SizePolicy::Fixed, PropertyViewUi::PropertySize);
  }

  void UpdateTransform() override
  {
    mBackground->SetSize(mSize);
    mLabel->SetSize(mSize);

    if (mExpandIcon->GetVisible())
    {
      mExpandIcon->SetTranslation(Vec3(0, 2, 0));

      const float cIconSpacing = Pixels(2);
      float labelPos = mExpandIcon->mSize.x + cIconSpacing;
      mLabel->SetTranslation(Vec3(labelPos, 0, 0));
    }

    float iconPos = mSize.x - mRemoveIcon->mSize.x;
    mRemoveIcon->SetTranslation(Vec3(iconPos, 2, 0));

    Composite::UpdateTransform();
  }

  void OnMouseEnterBackground(MouseEvent* event)
  {
    mBackground->SetColor(ComponentUi::TitleHighlight);
  }

  void OnMouseExitBackground(MouseEvent* event)
  {
    mBackground->SetColor(ComponentUi::TitleColor);
  }

  void OnMouseEnterRemove(MouseEvent* event)
  {
    mBackground->SetColor(ComponentUi::TitleRemove);
  }

  void OnMouseExitRemove(MouseEvent* event)
  {
    mBackground->SetColor(ComponentUi::TitleColor);
  }
};

class AddItemButton : public Composite
{
public:
  typedef AddItemButton ZilchSelf;

  Element* mBackground;
  Element* mBorder;
  Label* mLabel;

  AddItemButton(Composite* parent, StringParam itemName)
    : Composite(parent)
  {
    SetClipping(true);

    mBackground = CreateAttached<Element>(cWhiteSquare);
    mBackground->SetColor(ComponentUi::TitleColor);
    mBackground->MoveToBack();

    mBorder = CreateAttached<Element>(cWhiteSquareBorder);
    mBorder->SetColor(FloatColorRGBA(181, 103, 54, 255));
    mBorder->SetInteractive(false);

    mLabel = new Label(this);
    mLabel->SetInteractive(false);
    mLabel->SetText(BuildString("Add ", itemName, "..."));
    mLabel->SizeToContents();

    SetSizing(SizeAxis::Y, SizePolicy::Fixed, PropertyViewUi::PropertySize);

    ConnectThisTo(this, Events::MouseEnterHierarchy, OnMouseEnter);
    ConnectThisTo(this, Events::MouseExitHierarchy, OnMouseExit);
  }

  void UpdateTransform() override
  {
    mBackground->SetSize(mSize);
    mBorder->SetSize(mSize);

    float center = mSize.x * 0.5f;
    //float totalWidth = mIcon->mSize.x + Pixels(4) + mLabel->mSize.x;

    //float iconPos = Math::Max(center - totalWidth * 0.5f, 0.0f);
    Vec2 labelPos = mSize * 0.5f - mLabel->GetSize() * 0.5f;

    //mIcon->SetTranslation(Vec3(iconPos, 1, 0));
    mLabel->SetTranslation(ToVector3(labelPos));

    Composite::UpdateTransform();
  }

  void OnMouseEnter(MouseEvent* event)
  {
    mBackground->SetColor(ComponentUi::TitleHighlight);
  }

  void OnMouseExit(MouseEvent* event)
  {
    mBackground->SetColor(ComponentUi::TitleColor);
  }
};

class ItemStack : public Composite
{
public:

  Element* mBackground;

  ItemStack(Composite* parent, Vec2 spacing, Thickness padding)
    : Composite(parent)
  {
    mBackground = CreateAttached<Element>(cWhiteSquare);
    mBackground->SetColor(ComponentUi::BackgroundColor);
    mBackground->SetInteractive(false);
    mBackground->SetNotInLayout(true);

    SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, spacing, padding));
  }

  void UpdateTransform()
  {
    mBackground->SetSize(mSize);
    Composite::UpdateTransform();
  }
};

template <typename ResourceList>
class ResourceListOperation : public Operation
{
public:
  UndoHandle mObjectHandle;
  // Storing a pointer to this is safe because it only ever points to native types, which never
  // get destructed
  BoundTypeHandle mMeta;
  String mResourceIdName;
  uint mIndex;
  bool mAddOp;

  ResourceListOperation(HandleParam object, StringParam resourceIdName, uint index = -1, bool addOp = true)
    : mResourceIdName(resourceIdName)
    , mIndex(index)
    , mAddOp(addOp)
    , mObjectHandle(object)
  {
    mName = "ResourceListOperation";
    mMeta = object.StoredType;
  }

  void Undo() override
  {
    if (mAddOp)
      RemoveResource();
    else
      AddResource();
  }

  void Redo() override
  {
    if (mAddOp)
      AddResource();
    else
      RemoveResource();
  }

  void AddResource()
  {
    GetResourceList()->AddResource(mResourceIdName, mIndex);
    SendPropertyEvent();
  }

  void RemoveResource()
  {
    GetResourceList()->RemoveResource(mResourceIdName);
    SendPropertyEvent();
  }

  ResourceList* GetResourceList()
  {
    return mObjectHandle.Get<ResourceList*>();
  }

  void SendPropertyEvent()
  {
    Resource* resource = GetResourceList()->mOwner;

    MetaOperations::NotifyComponentsModified(resource);
  }
};

template <typename ResourceList>
class ResourceListItem : public ListItem
{
public:
  typedef ResourceListItem ZilchSelf;

  PropertyWidget* mPropertyWidget;
  ResourceList* mResourceList;
  String mResourceIdName;

  ResourceListItem(Composite* parent, PropertyWidget* propertyWidget, ResourceList* resourceList, StringParam listName, StringParam resourceIdName)
    : ListItem(parent, listName, false, !resourceList->GetReadOnly())
    , mPropertyWidget(propertyWidget)
    , mResourceList(resourceList)
    , mResourceIdName(resourceIdName)
  {
    ConnectThisTo(mBackground, Events::LeftClick, OnSelect);
    ConnectThisTo(mRemoveIcon, Events::LeftClick, OnRemove);
  }

  void OnSelect(MouseEvent* event)
  {
    Resource* resource = Z::gResources->GetResourceByName(mResourceIdName);
    if (resource != nullptr)
      Z::gEditor->EditResource(resource);
  }

  void OnRemove(MouseEvent* event)
  {
    uint index = mResourceList->GetResourceIndex(mResourceIdName);
    Operation* operation = new ResourceListOperation<ResourceList>(mResourceList, mResourceIdName, index, false);

    PropertyToUndo* undoProp = (PropertyToUndo*)mPropertyWidget->mProp;
    undoProp->mOperationQueue->Queue(operation);
    operation->Redo();
  }
};

template <typename ResourceList>
class ResourceListSearchProvider : public SearchProvider
{
public:
  ResourceList* mResourceList;

  ResourceListSearchProvider(ResourceList* resourceList)
    : mResourceList(resourceList)
  {
  }

  void Search(SearchData& search) override
  {
    Array<Resource*> resources;
    ResourceList::ManagerType::GetInstance()->EnumerateResources(resources);

    forRange (Resource* resource, resources.All())
    {
      SearchViewResult result;
      result.Data = resource;
      result.Interface = this;
      result.Name = resource->Name;
      result.Priority = 0;

      mResourceList->CheckForAddition(result.mStatus, resource);

      search.Results.PushBack(result);
    }
  }

  String GetType(SearchViewResult& element) override
  {
    return mResourceList->GetResourceTypeName();
  }
};

// Interface required of ResourceList:
// typedef ResourceManager ManagerType;
// String GetDisplayName();
// String GetResourceTypeName();
// bool GetReadOnly();
// bool GetExpanded();
// void SetExpanded(bool expanded);
// uint GetResourceIndex(StringParam resourceIdName);
// void CheckForAddition(Status& status, Resource* resource);
// void AddResource(StringParam resourceIdName, uint index);
// void RemoveResource(StringParam resourceIdName);
// Array<String>::range All();
template <typename ResourceList>
class ResourceListEditor : public PropertyWidget
{
public:
  typedef ResourceListEditor<ResourceList> ZilchSelf;

  ResourceList* mResourceList;

  ListItem* mTitle;
  ItemStack* mItemStack;
  HandleOf<FloatingSearchView> mActiveSearch;

  ResourceListEditor(PropertyWidgetInitializer& initializer)
    : PropertyWidget(initializer)
  {
    // No ui for multi selection
    if (initializer.Instance.Get<MetaSelection*>())
      return;

    mResourceList = initializer.Property->GetValue(initializer.Instance).Get<ResourceList*>();

    mDefSet = initializer.Parent->GetDefinitionSet();
    mLabel->SetActive(false);

    String titleName = mResourceList->GetDisplayName();
    if (titleName.Empty() == false)
    {
      mTitle = new ListItem(this, titleName, true, false, true);
      //if (mResourceList->GetReadOnly())
      //  mTitle->mLabel->SetColor(ToFloatColor(Color::Gray));
      ConnectThisTo(mTitle, Events::LeftClick, OnTitleClick);
    }

    SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 0), Thickness::cZero));
    SizeToContents();

    if (mResourceList->GetExpanded())
      Expand();
  }

  void OnTitleClick(MouseEvent* event)
  {
    mResourceList->SetExpanded(!mResourceList->GetExpanded());

    if (mResourceList->GetExpanded())
      Expand();
    else
      Close();
  }

  void Expand()
  {
    mTitle->mExpandIcon->ChangeDefinition(mDefSet->GetDefinition("PropArrowDown"));

    mItemStack = new ItemStack(this, Pixels(0, 2), Thickness(PropertyViewUi::IndentSize, 2, 2, 2));

    forRange (StringParam resourceIdName, mResourceList->All())
    {
      StringRange listName = resourceIdName.FindFirstOf(':');
      listName = StringRange(listName.End(), resourceIdName.End());
      ResourceListItem<ResourceList>* listItem = new ResourceListItem<ResourceList>(mItemStack, this, mResourceList, listName, resourceIdName);

      Resource* resource = ResourceList::ManagerType::FindOrNull(resourceIdName);
      if (resource == nullptr)
        listItem->mLabel->SetColor(ToFloatColor(Color::Gray));
    }

    if (!mResourceList->GetReadOnly())
    {
      AddItemButton* addButton = new AddItemButton(mItemStack, mResourceList->GetResourceTypeName());
      ConnectThisTo(addButton, Events::LeftClick, OnAddClick);
    }

    mItemStack->SizeToContents();
    SizeToContents();
    mGrid->MarkAsNeedsUpdate();
  }

  void Close()
  {
    mTitle->mExpandIcon->ChangeDefinition(mDefSet->GetDefinition("PropArrowRight"));

    mItemStack->Destroy();

    SizeToContents();
    mGrid->MarkAsNeedsUpdate();
  }

  void OnAddClick(MouseEvent* event)
  {
    Vec3 mousePos = ToVector3(event->GetMouse()->GetClientPosition());
    OpenSearch(mousePos);
  }

  FloatingSearchView* OpenSearch(Vec3 position)
  {
    FloatingSearchView* viewPopUp = new FloatingSearchView(this);
    viewPopUp->SetSize(Pixels(300,400));
    viewPopUp->UpdateTransformExternal();
    viewPopUp->ShiftOntoScreen(position);
    viewPopUp->UpdateTransformExternal();

    SearchView* searchView = viewPopUp->mView;
    SearchProvider* provider = new ResourceListSearchProvider<ResourceList>(mResourceList);
    searchView->mSearch->SearchProviders.PushBack(provider);
    searchView->Search(String());
    searchView->TakeFocus();

    mActiveSearch = viewPopUp;
    ConnectThisTo(searchView, Events::SearchCompleted, OnSearchCompleted);

    return viewPopUp;
  }

  void OnSearchCompleted(SearchViewEvent* event)
  {
    if (event->Element->mStatus.Succeeded())
    {
      Resource* resource = (Resource*)event->Element->Data;

      Operation* operation = new ResourceListOperation<ResourceList>(mResourceList, resource->ResourceIdName);

      PropertyToUndo* undoProp = (PropertyToUndo*)mProp;
      undoProp->mOperationQueue->Queue(operation);
      operation->Redo();
    }

    mActiveSearch.SafeDestroy();
  }
};

class CompositionLabel : public PropertyWidget
{
public:

  Element* mDivider;
  Element* mLabelBackground;
  Label* mLabel;

  CompositionLabel(PropertyWidgetInitializer& initializer)
    : PropertyWidget(initializer)
  {
    mDivider = CreateAttached<Element>(cWhiteSquare);
    mDivider->SetColor(PropertyViewUi::ColorWidgetHighlight);
    mDivider->SetInteractive(false);

    mLabelBackground = CreateAttached<Element>(cWhiteSquare);
    mLabelBackground->SetColor(MenuUi::BackgroundColor);
    mLabelBackground->SetInteractive(false);

    mLabel = new Label(this, "BoldText");
    mLabel->SetColor(Vec4(0.6f, 0.6f, 0.6f, 0.75f));
    mLabel->SetText("Composition");
    mLabel->SizeToContents();
  }

  void UpdateTransform()
  {
    mDivider->SetSize(Vec2(mSize.x, 2));
    mDivider->SetTranslation(Vec3(0, mSize.y * 0.5f - 1, 0));

    mLabelBackground->SetSize(mLabel->mSize);

    float labelPos = Math::Max(mSize.x * 0.5f - mLabel->mSize.x * 0.5f, 0.0f);
    mLabelBackground->SetTranslation(Vec3(labelPos, 0, 0));
    mLabel->SetTranslation(Vec3(labelPos, 0, 0));

    PropertyWidget::UpdateTransform();
  }
};

void RegisterEngineEditors()
{
  ZilchTypeId(CogPath)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorCogPath>));

  // METAREFACTOR Was this ever used?
  //PropEditors->PropertyEditorMap["Cog"] = new PropertyGridTypeData(&CreateProperty<PropertyEditorCogRef>);

  ZilchTypeId(Resource)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorResource>));
  ZilchTypeId(EditorResource)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorResource>));
  ZilchTypeId(CogArchetypeExtension)->Add(new MetaPropertyEditor(&CreateProperty<PropertyArchetype>));
  ZilchTypeId(RenderGroupList)->Add(new MetaPropertyEditor(&CreateProperty< ResourceListEditor<RenderGroupList> >));
  ZilchTypeId(MaterialList)->Add(new MetaPropertyEditor(&CreateProperty< ResourceListEditor<MaterialList> >));
  ZilchTypeId(CompositionLabelExtension)->Add(new MetaPropertyEditor(&CreateProperty<CompositionLabel>));
}

}//namespace Zero
