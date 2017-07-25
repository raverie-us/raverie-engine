///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationControls.cpp
/// Implementation of AnimationControls helper class.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace AnimTrackUi
{
const cstr cLocation = "EditorUi/AnimationEditor/TrackView";
Tweakable(Vec4, BackgroundColor,       Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ErrorToolTopBorder,    Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ErrorBgColorPrimary,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ErrorBgColorSecondary, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, DisabledText,          Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, HintColor,             Vec4(1,1,1,1), cLocation);
}

//------------------------------------------------------------------ Data Source
const u64 cRootId = (u64)-1;

class AnimationTrackSource : public DataSource
{
public:
  AnimationEditorData* mEditorData;
  RichAnimation* mRichAnimation;

  //****************************************************************************
  AnimationTrackSource(AnimationEditorData* editorData)
  {
    mEditorData = editorData;
    mRichAnimation = editorData->mRichAnimation;
  }

  //****************************************************************************
  //Data base Indexing
  DataEntry* GetRoot() override
  {
    return mRichAnimation->mRoot;
  }

  //****************************************************************************
  //Safe Indexing
  DataEntry* ToEntry(DataIndex index) override
  {
    return mRichAnimation->mTrackMap.FindValue(index.Id, nullptr);
  }

  //****************************************************************************
  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    TrackNode* trackInfo = (TrackNode*)dataEntry;
    return DataIndex(trackInfo->Id);
  }

  //****************************************************************************
  DataEntry* Parent(DataEntry* dataEntry) override
  {
    TrackNode* trackInfo = (TrackNode*)dataEntry;
    return trackInfo->Parent;
  }

  //****************************************************************************
  uint ChildCount(DataEntry* dataEntry) override
  {
    TrackNode* trackInfo = (TrackNode*)dataEntry;
    return trackInfo->Children.Size();
  }

  //****************************************************************************
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    TrackNode* trackInfo = (TrackNode*)dataEntry;
    return trackInfo->Children[index];
  }

  //****************************************************************************
  //Tree expanding
  bool IsExpandable(DataEntry* dataEntry) override
  {
    if(dataEntry == mRichAnimation->mRoot)
      return true;
    // If it has children, it's expandable
    TrackNode* trackInfo = (TrackNode*)dataEntry;
    return !trackInfo->Children.Empty();
  }

  //****************************************************************************
  //Data Base Cell Modification and Inspection
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    TrackNode* track = (TrackNode*)dataEntry;

    // For the name column, simply use the tracks name
    if(column == CommonColumns::Name)
    {
      variant = track;
    }
    else if(column == CommonColumns::ToolTip)
    {
      Cog* animGraphObject = mEditorData->mEditor->GetAnimationGraphObject();
      Status status;
      track->IsValid(animGraphObject, status);
      variant = status.Message;
    }
    else if(column == CommonColumns::Icon)
    {
      // If the selected object doesn't have the object (Cog, Component, 
      // or property) represented by the track, display an error icon
      Cog* animGraphObject = mEditorData->mEditor->GetAnimationGraphObject();
      Status status;
      TrackNode* invalidNode = track->IsValid(animGraphObject, status);

      if(status.Failed())
      {
        if(track == invalidNode)
          variant = String("InvalidObjectTrack");
        else
          variant = String("InvalidSubTrack");
      }
      else
      {
        // If it's a component track, use the name of the component because
        // it will map to an icon
        if(track->Type == TrackType::Component)
          variant = track->Name;
        // Object tracks will just use the Object icon
        else if(track->Type == TrackType::Object)
          variant = String("ItemIcon");
        // (Sub)Property tracks have no icon for now
        else
          variant = String("");
      }
    }
    else if(column == "TrackColor")
    {
      // If the track is selected, use its display color
      if(mEditorData->mVisiblePropertyTracks.Count(track) > 0)
      {
        Vec4 color;

        if(track->IsDisabled())
          color = Vec4(AnimTrackUi::DisabledText);
        else
          color = track->mDisplayColor;

        // Use the alpha of the actual color
        color.w = track->mDisplayColor.w;

        // Set the color
        variant = color;
      }
      // Otherwise set it to invisible
      else
        variant = Vec4::cZero;
    }
  }

  //****************************************************************************
  bool IsValid(DataEntry* dataEntry) override
  {
    Cog* animGraphObject = mEditorData->mEditor->GetAnimationGraphObject();
    Status status;
    TrackNode* track = (TrackNode*)dataEntry;
    track->IsValid(animGraphObject, status);

    return !status.Failed();
  }

  //****************************************************************************
  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    TrackNode* track = (TrackNode*)dataEntry;
    if(track->Type != TrackType::Object || track->IsRoot())
    {
      DoNotifyWarning("Failed to rename track", "Can only rename object tracks.");
      return false;
    }

    Status status;
    track->Rename(variant.Get<String>(), status);

    if(status.Failed())
    {
      DoNotifyWarning("Failed to rename track", status.Message);
      return true;
    }

    return true;
  }
};

//------------------------------------------------------- Animation Track Editor
/// Displays the color of the curve next to the name of the track
class AnimationNameEditor : public InPlaceTextEditor
{
public:
  //****************************************************************************
  AnimationNameEditor(Composite* parent, u32 flags) 
    : InPlaceTextEditor(parent, flags)
  {
    
  }

  //****************************************************************************
  void SetVariant(AnyParam variant) override
  {
    TrackNode* track = variant.Get<TrackNode*>();

    Any text;
    if(track->Name == "/")
      text = String("Root");
    else if(track->mDisabled)
      text = BuildString(track->Name, " (Disabled)");
    else
      text = track->Name;
    
    mText->mText->SetColor(Vec4(1,1,1,1));
    if(track->IsDisabled())
      mText->mText->SetColor(AnimTrackUi::DisabledText);

    InPlaceTextEditor::SetVariant(text);
  }

  //****************************************************************************
  void Edit()
  {
    // If it Contains the (Disabled) string, we need to remove it before editing
    String text = mText->GetText();

    StringRange found = text.FindFirstOf(" (Disabled)");
    if(!found.Empty())
      mText->SetText(text.SubString(text.Begin(), found.Begin()));
    InPlaceTextEditor::Edit();
  }
};

ValueEditor* CreateAnimationNameEditor(Composite* composite, AnyParam data, u32 flags)
{
  return new AnimationNameEditor(composite, flags);
}

//------------------------------------------------------- Animation Track Editor
/// Displays the color of the curve next to the name of the track
class AnimationTrackColor : public ValueEditor
{
public:
  Element* mColorBar;
  Vec4 mColor;

  //****************************************************************************
  AnimationTrackColor(Composite* parent) 
    : ValueEditor(parent)
  {
    static const String className = "AnimatorUI";
    mDefSet = mDefSet->GetDefinitionSet(className);

    mColorBar = CreateAttached<Element>("TrackColor");
  }

  //****************************************************************************
  void UpdateTransform() override
  {
    // Center the color bar
    const float cScalar = 0.25f;
    mColorBar->SetSize(SnapToPixels(Vec2(mSize.x * 0.8f, mSize.y * cScalar)));

    Vec3 position(0, (mSize.y * 0.5f) - (mSize.y * cScalar * 0.5f), 0);
    mColorBar->SetTranslation(SnapToPixels(position));

    Composite::UpdateTransform();
  }

  //****************************************************************************
  void SetVariant(AnyParam& variant) override
  {
    // It should be given to us as a color
    mColor = variant.Get<Vec4>();
    mColorBar->SetColor(mColor);
  }

  //****************************************************************************
  void GetVariant(Any& variant) override
  {
    variant = mColor;
  }
};

//******************************************************************************
ValueEditor* CreateTrackColor(Composite* parent, AnyParam data, u32 flags)
{
  return new AnimationTrackColor(parent);
}

void RegisterAnimationTrackViewEditors()
{
  // Register the color editor so that the tree view can create it
  ValueEditorFactory* factory = ValueEditorFactory::GetInstance();
  factory->RegisterEditor("AnimationTrackColor", CreateTrackColor);
  factory->RegisterEditor("AnimationName", CreateAnimationNameEditor);
}

//--------------------------------------------------------- Animation Track View
ZilchDefineType(AnimationTrackView, builder, type)
{
}

//******************************************************************************
AnimationTrackView::AnimationTrackView(Composite* parent, AnimationEditor* editor)
  : Composite(parent)
{
  mEditor = editor;
  mEditorData = nullptr;

  mSource = nullptr;
  // Create a filled background element
  mBackground = CreateAttached<Element>(cWhiteSquareWithBorder);
  mBackground->SetColor(AnimTrackUi::BackgroundColor);

  TreeFormatting formatting;

  // Icon column
  ColumnFormat* format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = CommonColumns::Icon;
  format->ColumnType = ColumnType::Fixed;
  format->FixedSize = Pixels(16, 20);
  format->Editable = false;
  format->CustomEditor = "IconEditor";

  // Name column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = CommonColumns::Name;
  format->ColumnType = ColumnType::Flex;
  format->FixedSize.y = Pixels(20);
  format->FlexSize = 1;
  format->HeaderName = "Track";
  format->Editable = true;
  format->CustomEditor = "AnimationName";

  // Curve Color column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = "TrackColor";
  format->ColumnType = ColumnType::Fixed;
  format->FixedSize = Pixels(30, 20);
  format->Editable = false;
  format->CustomEditor = "AnimationTrackColor";

  // Create the tree
  mTree = new TreeView(this);
  mTree->SetFormat(formatting);

  ConnectThisTo(mTree, Events::TreeRightClick, OnTreeRightClick);
}

//******************************************************************************
void AnimationTrackView::UpdateTransform()
{
  mBackground->SetSize(mSize);
  UpdateToolTip();

  Rect local = GetLocalRect();
  local.RemoveThickness(Thickness(1,1,1,1));
  PlaceWithRect(local, mTree);
  mTree->Refresh();
  Composite::UpdateTransform();
}

//******************************************************************************
void AnimationTrackView::SetAnimationEditorData(AnimationEditorData* editorData)
{
  mEditorData = editorData;

  RichAnimation* richAnimation = mEditorData->mRichAnimation;

  // We have to store the old data source because mTree->SetDataSource will
  // try to disconnect from it
  DataSource* oldSource = mSource;

  mSource = new AnimationTrackSource(mEditorData);
  DataSelection* selection = mTree->GetSelection();

  DisconnectAll(selection, this);
  DisconnectAll(editorData, this);
  DisconnectAll(richAnimation, this);

  ConnectThisTo(selection, Events::DataSelectionFinal, OnTracksSelected);
  ConnectThisTo(editorData, Events::TrackSelectionModified, OnSelectionModified);
  ConnectThisTo(mSource, Events::DataActivated, OnDataActivated);

  mTree->SetDataSource(mSource);

  // Now that the tree has disconnected from it, we can safely delete it
  SafeDelete(oldSource);

  ConnectThisTo(richAnimation, Events::TrackAdded, OnTrackAdded);

  // Expand all 
  DataSelection* expanded = mTree->GetExpanded();
  forRange(TrackNode* track, richAnimation->allTracks())
  {
    if(track->IsRoot() || track->Parent->IsRoot())
      expanded->Select(mSource->ToIndex(track));
    else if(track->Type != TrackType::Object)
      expanded->Select(mSource->ToIndex(track));
  }

  UpdateToolTip();
}

//******************************************************************************
void AnimationTrackView::SetSelection(Array<TrackNode*>& selection)
{
  // Clear the selection
  DataSelection* dataSelection = mTree->GetSelection();
  dataSelection->SelectNone();

  for(uint i = 0; i < selection.Size(); ++i)
  {
    DataIndex index = mSource->ToIndex(selection[i]);
    dataSelection->Select(index);
  }
}

//******************************************************************************
void AddPropertyTracks(TrackNode* root, HashSet<TrackNode*>& selection, 
                       bool childrenObjects)
{
  if(root->Type == TrackType::Property || root->Type == TrackType::SubProperty)
    selection.Insert(root);

  for(uint i = 0; i < root->Children.Size(); ++i)
  {
    TrackNode* curr = root->Children[i];
    if(curr->Type != TrackType::Object || childrenObjects)
      AddPropertyTracks(curr, selection, childrenObjects);
  }
}

//******************************************************************************
void AnimationTrackView::FocusOnObject(TrackNode* track)
{
  HashSet<TrackNode*> propertyTracks;
  AddPropertyTracks(track, propertyTracks, false);

  Array<TrackNode*> selection;
  forRange(TrackNode* info, propertyTracks.All())
  {
    selection.PushBack(info);
  }

  DataIndex index(track->Id);
  mTree->ShowRow(index);
  SetSelection(selection);

  mEditorData->SetSelection(selection);
}

//******************************************************************************
void AnimationTrackView::Hide()
{
  mTree->SetActive(false);
  mToolTip.SafeDestroy();
}

//******************************************************************************
void AnimationTrackView::Show()
{
  mTree->SetActive(true);

  // Will update the hint text to whether or not it should be visible
  MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationTrackView::UpdateToolTip()
{
  // If the tree is disabled (we're hidden)
  if(!mTree->GetActive())
    return;

  // The tooltip should only be visible if there are visible tracks
  bool active = false;
  if(mSource)
  {
    DataEntry* root = mSource->GetRoot();
    if(mSource->ChildCount(root) == 0)
      active = true;
  }

  // If it shouldn't be displayed, delete it if it's already there
  if(!active)
  {
    mToolTip.SafeDestroy();
    return;
  }

  // Create the tooltip if it isn't already there
  ToolTip* toolTip = mToolTip;
  if(toolTip == nullptr)
  {
    toolTip = new ToolTip(GetRootWidget());
    toolTip->SetText("Press the key icon next to a property in the property grid to create a key frame.");
    mToolTip = toolTip;
  }
    
  // Place the tooltip around the track view
  ToolTipPlacement placement;
  Rect rect = GetScreenRect();
  rect.SizeX -= Pixels(5);
  placement.SetScreenRect(rect);
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, 
                        IndicatorSide::Top, IndicatorSide::Bottom);
  toolTip->SetArrowTipTranslation(placement);
}

//******************************************************************************
void AnimationTrackView::OnSelectionModified(Event* e)
{
  // Clear the selection
  DataSelection* dataSelection = mTree->GetSelection();
  dataSelection->SelectNone();

  forRange(TrackNode* track, mEditorData->mVisiblePropertyTracks.All())
  {
    DataIndex index = mSource->ToIndex(track);
    dataSelection->Select(index);
  }

  // We want the tree to update to display the newly selected items
  mTree->Refresh();
}

//******************************************************************************
void AnimationTrackView::OnDataActivated(DataEvent* e)
{
  mEditor->GetGraph()->FocusOnSelectedCurves(IntVec2(1,1));
}

//******************************************************************************
void AnimationTrackView::OnTreeRightClick(TreeEvent* e)
{
  // The index of the row that was right clicked
  DataIndex rowIndex = e->Row->mIndex;

  // It's not valid to delete a sub property track
  TrackNode* track = (TrackNode*)(mSource->ToEntry(rowIndex));

  // Cannot delete root track
  if(track->IsRoot())
    return;

  // Create a context menu under the mouse
  ContextMenu* menu = new ContextMenu(e->Row);
  Mouse* mouse = Z::gMouse;;
  menu->SetBelowMouse(mouse, Pixels(0,0));
  
  if(track->Type == TrackType::Object)
  {
    ConnectMenu(menu, "Rename", OnRename);
  }

  if(track->Type == TrackType::SubProperty)
  {
    ConnectMenu(menu, "Clear Keys", OnClearKeys);
  }
  else
  {
    // Add an option to delete the track
    if(track->mDisabled)
    {
      ConnectMenu(menu, "Enable", OnToggleEnable);
    }
    else
    {
      ConnectMenu(menu, "Disable", OnToggleEnable);
    }

    ConnectMenu(menu, "Delete", OnDeleteTrack);
  }

  // Store a context
  mCommandIndex = rowIndex;
}

//******************************************************************************
void AnimationTrackView::OnRename(Event* e)
{
  TreeRow* row = mTree->FindRowByIndex(mCommandIndex);
  row->Edit(CommonColumns::Name);
}

//******************************************************************************
void AnimationTrackView::OnToggleEnable(Event* e)
{
  TrackNode* track = (TrackNode*)(mSource->ToEntry(mCommandIndex));
  track->mDisabled = !track->mDisabled;
  track->Modified();
  mTree->Refresh();
}

//******************************************************************************
void AnimationTrackView::OnClearKeys(Event* e)
{
  TrackNode* track = (TrackNode*)(mSource->ToEntry(mCommandIndex));
  track->ClearKeyFrames();
}

//******************************************************************************
void AnimationTrackView::OnDeleteTrack(Event* e)
{
  TrackNode* track = (TrackNode*)(mSource->ToEntry(mCommandIndex));
  track->Destroy();
}

//******************************************************************************
void AnimationTrackView::OnTracksSelected(Event* e)
{
  // Get the selection
  DataSelection* dataSelection = mTree->GetSelection();

  // Get the selected indices
  Array<DataIndex> selectedIndices;
  dataSelection->GetSelected(selectedIndices);

  // Build the track selection
  HashSet<TrackNode*> propertyTracks;

  for(uint i = 0; i < selectedIndices.Size(); ++i)
  {
    // Get the track info
    DataEntry* entry = mSource->ToEntry(selectedIndices[i]);
    TrackNode* trackInfo = (TrackNode*)entry;

    AddPropertyTracks(trackInfo, propertyTracks, true);
  }

  Array<TrackNode*> selection;
  forRange(TrackNode* info, propertyTracks.All())
  {
    selection.PushBack(info);
  }

  SetSelection(selection);
  mEditorData->SetSelection(selection);
}

//******************************************************************************
void AnimationTrackView::OnTrackAdded(TrackEvent* e)
{
  // We have to refresh the tree to make sure it creates a row for the new track
  mTree->Refresh();

  // Focus on the newly added row
  DataIndex index = mSource->ToIndex(e->mTrack);
  mTree->ShowRow(index);
}

}//namespace Zero
