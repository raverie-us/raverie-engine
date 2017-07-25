///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationEditor.cpp
/// Implementation of AnimationEditor Composite
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace AnimEditorUi
{
const cstr cLocation = "EditorUi/AnimationEditor";
Tweakable(float, ControlsWidth,           Pixels(238), cLocation);
Tweakable(float, ScrubberHeight,          Pixels(65), cLocation);
Tweakable(Vec4, InactiveKeyColor,         Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ActiveKeyColor,           Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, NegativeXColor,           Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, NegativeXHighlight,       Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ErrorStateColor,          Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ErrorStateHighlightColor, Vec4(1,1,1,1), cLocation);
}

//******************************************************************************
/// Callback for adding custom icons to the property grid
Widget* CreateKeyFrameIcon(Composite* parent, HandleParam object, 
                           Property* metaProperty, void* clientData)
{
  // Only add the key icon to objects with valid properties that are not 
  // on the Cog object (we only want component properties)
  if(metaProperty && object.Get<Component*>() != nullptr)
  {
    // Create the icon
    Widget* icon = new PropertyKeyIcon(parent, object, metaProperty,
                                       (AnimationEditor*)clientData);
    
    // Looks best translated down a bit
    icon->SetTranslation(Pixels(0,4,0));

    return icon;
  }
  return nullptr;
}

//******************************************************************************
Animation* FindFirstAnimationOnCog(Cog* cog)
{
  AnimationManager* animManager = AnimationManager::GetInstance();

  // Check all properties on all components of the object
  forRange(Component* component, cog->GetComponents())
  {
    BoundType* componentType = ZilchVirtualTypeId(component);
    forRange(Property* metaProperty, componentType->GetProperties())
    {
      // If it's an Animation, return it
      if(metaProperty->PropertyType == ZilchTypeId(Animation))
      {
        // Look up the animation by its name and id
        String animName = metaProperty->GetValue(component).ToString();
        Animation* animation = (Animation*)animManager->GetResource(animName,
                                                  ResourceNotFound::ReturnNull);

        // Only return if we found a valid animation, otherwise continue
        if(animation)
          return animation;
      }
    }
  }

  return nullptr;
}


//--------------------------------------------------------------- Key Frame Icon
//******************************************************************************
PropertyKeyIcon::PropertyKeyIcon(Composite* parent, HandleParam object,
                                 Property* metaProperty, AnimationEditor* editor)
  : Composite(parent), mEditor(editor)
{
  mComponentHandle = object;
  mProperty = metaProperty;

  mIcon = this->CreateAttached<Element>("AnimatorCreateKeyDisabled");

  // Don't add types that we cannot animate
  if(!ValidPropertyTrack(metaProperty))
    mIcon->SetActive(false);

  // If the property already has key frames, create the red key frame icon
  if(mEditor->HasKeyFrames(object, mProperty))
    mIcon->SetColor(AnimEditorUi::ActiveKeyColor);
  else
    mIcon->SetColor(AnimEditorUi::InactiveKeyColor);

  mIcon->SetColor(Vec4(1,1,1,1) * 0.9f);

  ConnectThisTo(mIcon, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(mIcon, Events::LeftClick, OnLeftClick);
  ConnectThisTo(mIcon, Events::MouseExit, OnMouseExit);

  // Size to the icon's size
  SetSize(mIcon->GetSize());

  mMouseOver = false;
}

//******************************************************************************
void PropertyKeyIcon::UpdateTransform()
{
  if(mComponentHandle.IsNull())
  {
    Composite::UpdateTransform();
    return;
  }

  float highlight = mMouseOver ? 1.2f : 1.0f;
  Vec4 color;

  if(mEditor->HasKeyFrames(mComponentHandle, mProperty))
    color = AnimEditorUi::ActiveKeyColor;
  else
    color = AnimEditorUi::InactiveKeyColor;

  color *= highlight;
  color = Math::Clamp(color, Vec4::cZero, Vec4(1,1,1,1));
  color.w = 1.0f;
  mIcon->SetColor(color);

  Composite::UpdateTransform();
}

//******************************************************************************
void PropertyKeyIcon::OnMouseEnter(Event* e)
{
  mMouseOver = true;
  MarkAsNeedsUpdate();
}

//******************************************************************************
void PropertyKeyIcon::OnLeftClick(Event* e)
{
  if(mComponentHandle.IsNull())
    return;

  // Create a key frame
  mEditor->CreateKeyFrameAtPlayHead(mProperty, mComponentHandle);
  MarkAsNeedsUpdate();
}

//******************************************************************************
void PropertyKeyIcon::OnMouseExit(Event* e)
{
  mMouseOver = false;
  MarkAsNeedsUpdate();
}

//------------------------------------------------------------- Animation Editor
ZilchDefineType(AnimationEditor, builder, type)
{
}

//******************************************************************************
AnimationEditor::AnimationEditor(Composite* parent)
  : Composite(parent), mGraphData(Vec2(60,20)), mOnionSkinning(this)
{
  mMainViewport = nullptr;
  static const String className = "AnimatorUI";
  mDefSet = mDefSet->GetDefinitionSet(className);

  mEditorData = nullptr;

  // Default to 30 fps
  mGraphData.mGridScale = Vec2(30.0f, 1.0f);

  // Random values that just look good by default
  mGraphData.ZoomMin = Vec2(0.15f, 0.0001f);
  mGraphData.ZoomMax = Vec2(200.0f, 1000.0f);
  mGraphData.SetZoom(Vec2(25.0f, 1.8f));
  mGraphData.Translation = Vec2(0, 4.5f);

  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness::cZero));

  Composite* toolArea = new Composite(this);
  toolArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(20));
  toolArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(2,0), Thickness::cZero));
  {
    mSelector = new AnimationSelector(toolArea, this);
    mSelector->SetSizing(SizeAxis::X, SizePolicy::Fixed, AnimEditorUi::ControlsWidth);

    mToolBox = new AnimationToolBox(toolArea, this);
    mToolBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  }

  Spacer* spacer = new Spacer(this);
  spacer->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(4));

  Composite* topArea = new Composite(this);
  topArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  topArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(2,0), Thickness(Pixels(0,-1,0,0))));
  {
    // Create the track view
    mTrackView = new AnimationTrackView(topArea, this);
    mTrackView->SetSizing(SizeAxis::X, SizePolicy::Fixed, AnimEditorUi::ControlsWidth);
    mSettingsView = new AnimationSettingsView(topArea, this);
    mSettingsView->SetSizing(SizeAxis::X, SizePolicy::Fixed, AnimEditorUi::ControlsWidth);
    mSettingsView->SetActive(false);

    // Create the graph
    mGraph = new AnimationGraphEditor(topArea, this, &mGraphData);
    mGraph->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    //mDopeSheet = new AnimationDopeSheet(this, mScrubber);
  }

  Composite* bottomArea = new Composite(this);
  bottomArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, AnimEditorUi::ScrubberHeight);
  bottomArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(2,0), Thickness(Pixels(0,-1,0,0))));
  {
    // Create the controls
    mControls = new AnimationControls(bottomArea, this);
    mControls->SetSizing(SizeAxis::X, SizePolicy::Fixed, AnimEditorUi::ControlsWidth);
    //ConnectThisTo(mControls->mButtonSettings, Events::LeftClick, OnSettingsPressed);

    // Create and connect to the scrubber (we want to know when play head has changed)
    mScrubber = new AnimationScrubber(bottomArea, this, &mGraphData);
    mScrubber->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    ConnectThisTo(mScrubber, Events::PlayHeadModified, OnPlayHeadModified);
  }

  mControls->SetScrubber(mScrubber);
  mGraph->SetScrubber(mScrubber);
  mGraph->SetToolBox(mToolBox);

  ConnectThisTo(mScrubber->mNegativeArea, Events::LeftClick, OnMouseDownNegativeX);
  ConnectThisTo(mGraph->mNegativeArea, Events::LeftClick, OnMouseDownNegativeX);

  // Initialize the error state text
  mErrorStateText = new Label(mScrubber, "MessageText");
  mErrorStateText->SetNotInLayout(true);
  ConnectThisTo(mErrorStateText, Events::MouseEnter, OnMouseEnterErrorText);
  ConnectThisTo(mErrorStateText, Events::LeftMouseDown, OnMouseDownErrorText);
  ConnectThisTo(mErrorStateText, Events::MouseExit, OnMouseExitErrorText);

  // Default to no object selected
  SetErrorState(ErrorState::NoObject);

  // We want to connect to events for when the selection changes
  MetaSelection* selection = Z::gEditor->GetSelection();
  ConnectThisTo(selection, Events::SelectionChanged, OnSelectionChanged);
  ConnectThisTo(Z::gEditor, Events::Save, OnSave);
  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::KeyRepeated, OnKeyRepeated);
}

//******************************************************************************
AnimationEditor::~AnimationEditor()
{
  // Clean up all cached data
  forRange(AnimationEditorData* cachedData, mCachedAnimationData.Values())
    delete cachedData;
}

//******************************************************************************
void AnimationEditor::UpdateTransform()
{
  mErrorStateText->SizeToContents();
  Rect rect = mScrubber->GetLocalRect();
  PlaceCenterToRect(rect, mErrorStateText);

  // Update the color and position of the negative area
  UpdateNegativeArea();

  Composite::UpdateTransform();
}

//******************************************************************************
void AnimationEditor::SetPropertyView(MainPropertyView* view)
{
  mPropertyView = view;
  ConnectThisTo(view, Events::ComponentsModified, OnComponentsChanged);
  ConnectThisTo(view, Events::PropertyContextMenu, OnPropertyContextMenu);
}

//******************************************************************************
void AnimationEditor::SetAnimation(Animation* animation)
{
  if(Cog* animGraphObject = mAnimGraphObject)
  {
    SimpleAnimation* simpleAnim = animGraphObject->has(SimpleAnimation);

    if(simpleAnim == nullptr)
    {
      if(animGraphObject->AddComponentByType(ZilchTypeId(SimpleAnimation)))
        simpleAnim = animGraphObject->has(SimpleAnimation);
    }
    ReturnIf(simpleAnim == nullptr, , "Could not add SimpleAnimation");
    simpleAnim->SetAnimation(animation);
    RefreshSelection();
  }
}

//******************************************************************************
void AnimationEditor::SetStatusText(StringParam text, ByteColor color)
{
  mToolBox->SetStatusText(text, color);
}

//******************************************************************************
ErrorState::Type AnimationEditor::GetErrorState()
{
  return mErrorState;
}

//******************************************************************************
bool AnimationEditor::TakeFocusOverride()
{
  this->HardTakeFocus();
  MetaSelection* selection = Z::gEditor->GetSelection();
  Cog* primary = selection->GetPrimaryAs<Cog>();
  ObjectSelected(primary);
  return true;
}

//******************************************************************************
void AnimationEditor::UpdateToScrubber()
{
  UpdateToTime(mScrubber->GetPlayHead());
}

//******************************************************************************
Cog* AnimationEditor::GetAnimationGraphObject()
{
  return mAnimGraphObject;
}

//******************************************************************************
Cog* AnimationEditor::GetSelectedObject()
{
  return mSelectedObject;
}

//******************************************************************************
bool AnimationEditor::HasKeyFrames(HandleParam componentInstance,
                                   Property* metaProperty)
{
  if(mEditorData == nullptr)
    return false;

  RichAnimation* richAnimation = mEditorData->mRichAnimation;

  // Grab the component and object from the instance
  Component* component = componentInstance.Get<Component*>();
  if(component == nullptr)
    return false;
  Cog* object = component->GetOwner();

  // Find the object track (it will be created if it doesn't exist)
  String objectPath = GetObjectPath(object, mAnimGraphObject);
  TrackNode* objectTrack = richAnimation->GetObjectTrack(objectPath, false);

  if(objectTrack == nullptr)
    return false;

  // Find the property track (it will be created if it doesn't exist)
  String propertyPath = GetPropertyPath(component, metaProperty);
  TrackNode* propertyTrack = richAnimation->GetPropertyTrack(objectTrack,
                                   propertyPath, componentInstance.StoredType, false);
  return propertyTrack != nullptr;
}

//******************************************************************************
AnimationScrubber* AnimationEditor::GetScrubber()
{
  return mScrubber;
}

//******************************************************************************
AnimationGraphEditor* AnimationEditor::GetGraph()
{
  return mGraph;
}

//******************************************************************************
AnimationSettings* AnimationEditor::GetSettings()
{
  return &mSettings;
}

//******************************************************************************
AnimationEditorData* AnimationEditor::GetEditorData()
{
  return mEditorData;
}

//******************************************************************************
void AnimationEditor::UpdateNegativeArea()
{
  Widget* graphArea = mGraph->mNegativeArea;
  Widget* scrubberArea = mScrubber->mNegativeArea;

  // The width of the negative area
  float width = mGraphData.ToPixels(Vec2::cZero).x;

  // Expand it by the gutter area
  width = Math::Max(0.0f, width + Pixels(30));

  graphArea->SetSize(Vec2(width, mGraph->GetSize().y));
  scrubberArea->SetSize(Vec2(width, mScrubber->GetSize().y));

  // When only the gutter is visible, we want the negative area's color to
  // fade out
  float percentVisible = (width - Pixels(30)) / Pixels(30);
  percentVisible = Math::Clamp(percentVisible, 0.0f, 1.0f);

  // Start out as invisible
  Vec4 startColor = AnimEditorUi::NegativeXColor;
  startColor.w = 0.0f;

  // If the mouse is over either controls, interpolate to the highlighted color
  Vec4 endColor = AnimEditorUi::NegativeXColor;
  if(graphArea->IsMouseOver() || scrubberArea->IsMouseOver())
    endColor = AnimEditorUi::NegativeXHighlight;

  // Compute and set the final color
  Vec4 finalColor = Math::Lerp(startColor, endColor, percentVisible);
  graphArea->SetColor(finalColor);
  scrubberArea->SetColor(finalColor);
}

//******************************************************************************
Cog* FindAnimationGraphObject(Cog* object)
{
  if(object == nullptr)
    return nullptr;
  if(object->has(AnimationGraph))
    return object;
  return FindAnimationGraphObject(object->GetParent());
}

//******************************************************************************
void AnimationEditor::OnSelectionChanged(SelectionChangedEvent* event)
{
  // Do nothing if we're not active
  if(!GetActive())
    return;

  Cog* primary = event->Selection->GetPrimaryAs<Cog>();
  ObjectSelected(primary);
}

//******************************************************************************
void AnimationEditor::ObjectSelected(Cog* cog)
{
  mOnionSkinning.Clear();

  mPropertyView->GetPropertyView()->RemoveCustomPropertyIcon(&CreateKeyFrameIcon);

  // Do nothing if we're not active
  if(!GetActive())
    return;

  // If it's the same object as before, we're refreshing
  bool refreshing = (cog == mSelectedObject);

  // If we're refreshing, we don't need to re-connect everything
  if(!refreshing)
  {
    if(Cog* oldSelectedObject = mSelectedObject)
      oldSelectedObject->GetReceiver()->Disconnect(this);
  }

  if(cog)
  {
    // Set the new selected object
    mSelectedObject = cog;

    // Attempt to find an object with a animGraph
    mAnimGraphObject = FindAnimationGraphObject(cog);
  }
  else
  {
    mSelectedObject = cInvalidCogId;
    SetErrorState(ErrorState::NoObject);
    return;
  }


  // Connect to get property changed events only if we're not refreshing.
  // If we are refreshing, we should already be connected
  if(!refreshing)
  {
    ConnectThisTo(cog, Events::PropertyModified, OnPropertyChanged);

    // We need to connect to keyboard events on the viewport of the selected
    // object, so attempt to find it
    Widget* widget = Z::gEditor->mManager->FindWidgetWith(cog);
    ErrorIf(widget == nullptr, "Couldn't find window containing object.");

    // No need to do anything if the last object was in the same viewport
    if((Viewport*)mMainViewport != widget)
    {
      // If there was a previous viewport, we no longer care about
      // input from it
      if(Viewport* viewport = mMainViewport)
        viewport->GetReceiver()->Disconnect(this);

      // Set the new viewport
      EditorViewport* editorViewport = Type::DynamicCast<EditorViewport*>(widget);
      ErrorIf(editorViewport == nullptr, "Widget found was not an EditorViewport.");

      Viewport* viewport = editorViewport->GetReactiveViewport();

      // Connect to keyboard events
      ConnectThisTo(viewport, Events::KeyDown, OnKeyDownViewport);
      mMainViewport = viewport;
    }
  }

  // If there is no animGraph in the selection, we're in an invalid state
  if(!mAnimGraphObject.IsValid())
  {
    // If we have an object selected,
    if(mSelectedObject.IsValid())
      SetErrorState(ErrorState::NoAnimGraph);
    else
      SetErrorState(ErrorState::NoObject);

    // Invalidate the animation handle
    mAnimation = nullptr;
    return;
  }

  // Get the animation from the animGraph
  Cog* animGraphCog = mAnimGraphObject;
  Animation* animation = nullptr;

  // First check for the animation on the SimpleAnimation component
  if(SimpleAnimation* autoPlay = animGraphCog->has(SimpleAnimation))
    animation = autoPlay->GetAnimation();

  // If we didn't find an animation, attempt to find it on any other component
  if(animation == nullptr)
    animation = FindFirstAnimationOnCog(animGraphCog);

  // We cannot edit non-writable animations
  if(animation == nullptr || !animation->IsWritable())
  {
    SetErrorState(ErrorState::ReadOnly);

    // Invalidate the animation handle
    mAnimation = nullptr;
    return;
  }

  // If it comes from geometry content, we would need to copy it
  if(Type::DynamicCast<GeometryContent*>(animation->mContentItem))
  {
    SetErrorState(ErrorState::AnimationFromGeometry);

    mAnimation = animation;
    return;
  }

  mPropertyView->GetPropertyView()->AddCustomPropertyIcon(&CreateKeyFrameIcon, this);

  // We have a valid object
  SetErrorState(ErrorState::None);

  // Do nothing if we already have this animation selected
  if((Animation*)mAnimation != animation)
  {
    // Set the animation
    mAnimation = animation;

    // Get the editor data from the animation
    mEditorData = GetEditorData(animGraphCog, animation);

    RichAnimation* richAnim = mEditorData->mRichAnimation;
    ConnectThisTo(richAnim, Events::AnimationModified, OnAnimationModified);

    mTrackView->SetAnimationEditorData(mEditorData);
    mScrubber->SetAnimationEditorData(mEditorData);
    mGraph->SetAnimationEditorData(mEditorData);
    mControls->SetAnimationEditorData(mEditorData);
    mSettingsView->SetAnimationEditorData(mEditorData);
  }

  ShowControls();

  // Find and focus on the object track
  String objectPath = GetObjectPath(mSelectedObject, mAnimGraphObject);
  RichAnimation* richAnimation = mEditorData->mRichAnimation;
  TrackNode* objectTrack = richAnimation->GetObjectTrack(objectPath, false);
  if(objectTrack && !refreshing)
    mTrackView->FocusOnObject(objectTrack);

  // Update the scrubber
  UpdateToTime(mScrubber->GetPlayHead());
}

//******************************************************************************
AnimationEditorData* AnimationEditor::GetEditorData(Cog* animGraphCog,
                                                    Animation* animation)
{
  ResourceId animationId = animation->mResourceId;

  // Check to see if it has already been created
  AnimationEditorData* data = mCachedAnimationData.FindValue(animationId, nullptr);
  if(data == nullptr)
  {
    data = new AnimationEditorData(this, animGraphCog, animation, &mGraphData);
    mCachedAnimationData.Insert(animationId, data);
  }
  return data;
}

//******************************************************************************
void AnimationEditor::HideControls()
{
  // No need to do anything it there is no valid animation anyways
  if(mEditorData == nullptr)
    return;

  mControls->Hide();
  mScrubber->Hide();
  mGraph->Hide();
  mTrackView->Hide();
  mSelector->Hide();
  mToolBox->Hide();
}

//******************************************************************************
void AnimationEditor::ShowControls()
{
  // No need to do anything it there is no valid animation anyways
  if(mEditorData == nullptr)
    return;

  mControls->Show();
  mScrubber->Show();
  mGraph->Show();
  mTrackView->Show();
  mSelector->Show();
  mToolBox->Show();
}

//******************************************************************************
void AnimationEditor::RefreshSelection()
{
  ObjectSelected(mSelectedObject);
}

//******************************************************************************
void AnimationEditor::OnAnimationModified(Event* e)
{
  TabModifiedEvent eventToSend(true);
  GetDispatcher()->Dispatch(Events::TabModified, &eventToSend);
}

//******************************************************************************
void AnimationEditor::OnSave(Event* e)
{
  if(mEditorData)
  {
    mEditorData->SaveRichAnimation();

    TabModifiedEvent eventToSend(false);
    GetDispatcher()->Dispatch(Events::TabModified, &eventToSend);
  }
}

//******************************************************************************
void AnimationEditor::OnPropertyChanged(PropertyEvent* event)
{
  // Do nothing if we're not active
  if(!GetActive())
    return;

  // No need to do anything if the animGraph isn't valid
  if(!mAnimGraphObject.IsValid())
    return;

  // If the current animation was set on the SimpleAnimation component, we want
  // to switch to editing that animation instead of creating a key frame
  // for changing the animation while an animation is playing
  BoundType* objectType = Type::GetBoundType(event->mObject.StoredType);
  String componentName = objectType->Name;

  Property* property = event->mProperty.GetPropertyFromRoot(event->mObject);
  String propertyName = property->Name;

  if(componentName == "SimpleAnimation")
  {
    if(propertyName == "Animation")
    {
      // We need to set the Animation because the it isn't set until
      // after we get this event
      Animation* animation = event->mNewValue.Get<Animation*>();

      if(animation)
        SetAnimation(animation);

      // Refresh the selection to edit the new animation
      RefreshSelection();
    }

    return;
  }

  // Validate the animation
  Animation* animation = mAnimation;
  if(animation == nullptr)
    return;

  // Do nothing if auto-key is disabled
  if(!mSettings.mAutoKey)
    return;

  // Create the key frame at the play head
  CreateKeyFrameAtPlayHead(property, Handle(event->mObject), &event->mNewValue);
}

//******************************************************************************
void AnimationEditor::OnComponentsChanged(PropertyEvent* event)
{
  // Do nothing if we're not active
  if(GetActive() == false)
    return;

  // Only make a change if the object changed is a cog
  if(Cog* object = event->mObject.Get<Cog*>())
  {
    // If it was the selected object that changed, refresh the selection
    // in case the AnimationGraph was added / removed
    if(object == mSelectedObject)
      RefreshSelection();
  }
}

//******************************************************************************
void AnimationEditor::OnPlayHeadModified(Event* event)
{
  UpdateToScrubber();
}

//******************************************************************************
void AnimationEditor::UpdateToTime(float t)
{
  if(Cog* cog = mAnimGraphObject)
  {
    AnimationGraph* animGraph = cog->has(AnimationGraph);

    if(animGraph)
    {
      AnimationNode* node = BuildBasic(animGraph, mAnimation, t, AnimationPlayMode::PlayOnce);
      animGraph->SetActiveNode(node);
      animGraph->ForceUpdate();
    }
  }
}

//******************************************************************************
void AnimationEditor::SetErrorState(ErrorState::Type state)
{
  // Set the state
  mErrorState = state;

  // Make the error text visible and interactive if the state is not 'None'
  bool active = (state != ErrorState::None);
  mErrorStateText->SetActive(active);
  mErrorStateText->SetInteractive(active);
  SetStatusText(String());

  // Hide all controls if there's an error state
  if(active)
    HideControls();

  // Set the error text based on the state
  if(state == ErrorState::NoObject)
  {
    mErrorStateText->SetText("No Object Selected");
    SetStatusText("Select an object");
  }
  else if(state == ErrorState::NoAnimGraph)
  {
    mErrorStateText->SetText("Click to add Animation Graph +");
    SetStatusText("No AnimationGraph");
  }
  else if(state == ErrorState::AnimationFromGeometry)
  {
    mErrorStateText->SetText("Convert to Rich Animation +");
    SetStatusText("Invalid Animation source");
  }
  else if(state == ErrorState::ReadOnly)
  {
    mErrorStateText->SetText("Click to create new Animation +");
    SetStatusText("No Animation selected");
  }

  // Reset the color of the text
  mErrorStateText->SetColor(AnimEditorUi::ErrorStateColor);

  MarkAsNeedsUpdate();
}

//******************************************************************************
TrackNode* FindTranslationTrack(TrackNode* root)
{
  if(root->Path == "Transform.Translation")
    return root;

  forRange(TrackNode* child, root->Children.All())
  {
    TrackNode* ret = FindTranslationTrack(child);
    if(ret)
      return ret;
  }
  return nullptr;
}

//******************************************************************************
void AnimationEditor::OnUpdate(UpdateEvent* e)
{
  // Flush any changes to the animation
  if(mEditorData)
  {
    // Flush any changes made on the rich animation
    mEditorData->mRichAnimation->Flush();

    // Update the grid scale based on the fps
    float editFps = mSettings.mEditFps;
    mEditorData->mGraphData->mGridScale = Vec2(editFps, 1.0f);

    mScrubber->MarkAsNeedsUpdate();
  }

  // Update the auto key highlight
  SendHighlightEvent(e->Dt);

  mOnionSkinning.Update();
}

//******************************************************************************
void AnimationEditor::SendHighlightEvent(float dt)
{
  // Create the event to send
  HighlightBorderEvent eventToSend;

  // Default to deactivated
  eventToSend.mState = false;

  // If there are no current errors, query to see if auto key is on
  if(mErrorState == ErrorState::None)
    eventToSend.mState = mSettings.mAutoKey;

  // Flashing Red color
  static float sTest = 0.0f;
  eventToSend.mColor = Vec4(Math::Cos(sTest) * 0.2f + 0.8f, 0, 0, .5f);
  sTest += 0.03f;

  // Highlight both the property grid and the main view-port
  eventToSend.mWidget = mPropertyView;
  mPropertyView->DispatchBubble(Events::HighlightBorder, &eventToSend);

  // Find and highlight the window containing the selected object
  if(Cog* selected = mSelectedObject)
  {
    if(Viewport* viewport = mMainViewport)
    {
      eventToSend.mWidget = viewport;
      viewport->DispatchBubble(Events::HighlightBorder, &eventToSend);
    }
  }

  // If we are in auto key mode, display AutoKey text in the error state text
  if(eventToSend.mState)
  {
    mErrorStateText->SetText("Auto Key");
    mErrorStateText->SetColor(Vec4(1,0,0,0.15f));

    MarkAsNeedsUpdate();
  }

  // Update the visibility of the error text
  if(mErrorState == ErrorState::None)
    mErrorStateText->SetActive(eventToSend.mState);
}

//******************************************************************************
void AnimationEditor::OnKeyDownViewport(KeyboardEvent* e)
{
  // Do nothing if we're not active or we're in an error state
  if(!GetActive() || mErrorState != ErrorState::None)
    return;

  if(e->Key == Keys::K)
  {
    // We want to create a key frame at the play head
    float playHead = mScrubber->GetPlayHead();

    bool validTrackFound = false;
    forRange(TrackNode* track, mEditorData->mVisiblePropertyTracks.All())
    {
      if(track->Type != TrackType::Property && track->Type != TrackType::SubProperty)
        continue;

      // Query the value
      Any currentValue = track->SampleObject(mAnimGraphObject);

      // Create a key frame at the current play head
      // This can be invalid if the 
      if(currentValue.IsNotNull())
      {
        track->UpdateOrCreateKeyAtTime(playHead, currentValue);
        validTrackFound = true;
      }
    }

    if(validTrackFound == false)
      DoNotifyWarning("Cannot Create Key Frame", "Cannot create a key frame "
                      "with no tracks selected.");
    e->Handled = true;
  }
}

//******************************************************************************
void AnimationEditor::OnKeyDown(KeyboardEvent* e)
{
  if(mEditorData == nullptr)
    return;

  if(e->Key == Keys::F)
  {
    IntVec2 axes(1,1);

    // Only zoom on the x-axis is the mouse is over the scrubber
    if(mScrubber->IsMouseOver())
      axes.y = 0;
    // Only zoom on the y-axis if the mouse is over the negative area in the graph
    else if(mGraph->mNegativeArea->IsMouseOver())
      axes.x = 0;

    mGraph->FocusOnSelectedCurves(axes);
  }
  else
  {
    OnKeyDownViewport(e);
  }

  UpdateArrowMovement(e);
}

//******************************************************************************
void AnimationEditor::OnKeyRepeated(KeyboardEvent* e)
{
  UpdateArrowMovement(e);
}

//******************************************************************************
void AnimationEditor::UpdateArrowMovement(KeyboardEvent* e)
{
  float scrollSpeed = Pixels(5);
  if(e->ShiftPressed)
    scrollSpeed = Pixels(25);

  Vec2 movement = Vec2::cZero;
  if(e->Key == Keys::Left)
    movement.x += 1.0f;
  if(e->Key == Keys::Right)
    movement.x -= 1.0f;
  if(e->Key == Keys::Up)
    movement.y -= 1.0f;
  if(e->Key == Keys::Down)
    movement.y += 1.0f;

  mGraphData.ScrollPixels(movement * scrollSpeed);
}

//******************************************************************************
void AnimationEditor::OnPropertyContextMenu(ContextMenuEvent* e)
{
  if(mErrorState == ErrorState::None)
  {
    // Do nothing if the property is not supported by the animation system
    if(!ValidPropertyTrack(e->mProperty))
      return;

    mContextMenuProperty = e->mProperty;
    mContextMenuInstance = e->mInstance;
    ConnectMenu(e->mMenu, "Key Frame", OnCreateKeyFrame);
  }
}

//******************************************************************************
void AnimationEditor::OnCreateKeyFrame(Event* e)
{
  // Create a key frame. Pass in NULL for the value to let it query the object
  CreateKeyFrameAtPlayHead(mContextMenuProperty, mContextMenuInstance, nullptr);
}

//******************************************************************************
void AnimationEditor::OnMouseEnterErrorText(MouseEvent* event)
{
  if(mErrorState != ErrorState::NoObject)
    mErrorStateText->SetColor(AnimEditorUi::ErrorStateHighlightColor);

  mErrorToolTip.SafeDestroy();
  ToolTip* toolTip = new ToolTip(this);

  ToolTipPlacement placement;
  placement.SetScreenRect(mErrorStateText->GetScreenRect());
  
  placement.SetPriority(IndicatorSide::Top, IndicatorSide::Bottom,
                        IndicatorSide::Left, IndicatorSide::Right);
  mErrorToolTip = toolTip;

  if(mErrorState == ErrorState::NoObject)
    toolTip->SetText("Select an object in the level to add or edit an animation");
  else if(mErrorState == ErrorState::NoAnimGraph)
    toolTip->SetText("The AnimationGraph component is responsible for running Animations on objects.");
  else if(mErrorState == ErrorState::ReadOnly)
    toolTip->SetText("Click to create a new Rich Animation");
  else if(mErrorState == ErrorState::ReadOnly)
    toolTip->SetText("Click to convert the animation to a Rich Animation format");

  toolTip->SetArrowTipTranslation(placement);
}

//******************************************************************************
void AnimationEditor::OnMouseDownErrorText(MouseEvent* event)
{
  mErrorToolTip.SafeDestroy();

  bool hasEditorConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig) != nullptr;

  if(mErrorState == ErrorState::NoAnimGraph)
  {
    // Add a animGraph to the selected object
    Cog* cog = mSelectedObject;
    cog->AddComponentByName("AnimationGraph");

    // We need to tell the property grid that something changed on the object
    mPropertyView->GetPropertyView()->Invalidate();

    // Refresh the selection
    RefreshSelection();

    // The mouse should still be over the error state text, so 
    // keep it highlighted
    if(mErrorState == ErrorState::ReadOnly)
      mErrorStateText->SetColor(AnimEditorUi::ErrorStateHighlightColor);
  }
  else if(mErrorState == ErrorState::ReadOnly)
  {
    OpenAnimationAddWindow();
  }
  else if(mErrorState == ErrorState::AnimationFromGeometry && hasEditorConfig)
  {
    Animation* oldAnimation = mAnimation;

    // For now, create a new animation and Append 'Rich' to the front
    String newName = BuildString("Rich", oldAnimation->Name);

    // First check to see if we already created it
    Animation* newAnimation = AnimationManager::FindOrNull(newName);

    // If we didn't create it
    if(newAnimation == nullptr)
    {
      ResourceAdd resourceAdd;
      resourceAdd.Library = Z::gEditor->mProjectLibrary;
      resourceAdd.Name = newName;
      resourceAdd.Template = AnimationManager::FindOrNull("RichAnimation");
      newAnimation = (Animation*)AddNewResource(AnimationManager::GetInstance(), resourceAdd);
    }

    // Convert the animation from geometry to a rich animation
    RichAnimation* richAnimation = ConvertToRichAnimation(oldAnimation);
    String file = newAnimation->mContentItem->GetFullPath();

    // Save over the default rich animation that was created
    SaveToDataFile(*richAnimation, file);

    // Bake it to the new animation resource
    richAnimation->BakeToAnimation(newAnimation);

    Cog* cog = mAnimGraphObject;
    // Set the new animation on the AutoPlayAnimation and refresh
    SimpleAnimation* autoPlay = cog->has(SimpleAnimation);
    if(autoPlay == nullptr)
    {
      if(cog->AddComponentByType(ZilchTypeId(SimpleAnimation)))
        autoPlay = cog->has(SimpleAnimation);
    }

    // Set the animation
    autoPlay->SetAnimation(newAnimation);
    RefreshSelection();
  }
}

//******************************************************************************
void AnimationEditor::OnMouseExitErrorText(MouseEvent* event)
{
  mErrorToolTip.SafeDestroy();
  mErrorStateText->SetColor(AnimEditorUi::ErrorStateColor);
}

//******************************************************************************
void AnimationEditor::OnMouseDownNegativeX(MouseEvent* event)
{
  mGraphData.PanToTranslation(Vec2(0, mGraphData.Translation.y), 0.4f);
}

//******************************************************************************
void AnimationEditor::OpenAnimationAddWindow()
{
  AddResourceWindow* addWidget = OpenAddWindow(ZilchTypeId(Animation));
  addWidget->ShowResourceTypeSearch(false);

  // We want events back to know what happened with the Add dialog
  ConnectThisTo(addWidget, Events::AddWindowCancelled, OnAnimationAddCancel);
  ConnectThisTo(Z::gResources, Events::ResourceAdded, OnAnimationAdded);
}

//******************************************************************************
void AnimationEditor::OnAnimationAdded(ResourceEvent* e)
{
  Z::gResources->GetReceiver()->Disconnect(this);

  Cog* cog = mAnimGraphObject;
  if(cog && (ZilchVirtualTypeId(e->EventResource) == ZilchTypeId(Animation)))
  {
    // Set the new animation on the AutoPlayAnimation and refresh
    SimpleAnimation* autoPlay = cog->has(SimpleAnimation);
    if(autoPlay == nullptr)
    {
      if(cog->AddComponentByType(ZilchTypeId(SimpleAnimation)))
        autoPlay = cog->has(SimpleAnimation);
    }
    if(autoPlay)
      autoPlay->SetAnimation((Animation*)e->EventResource);
    RefreshSelection();
  }
}

//******************************************************************************
void AnimationEditor::OnAnimationAddCancel(Event* e)
{
  Z::gResources->GetReceiver()->Disconnect(this);
}

//******************************************************************************
void AnimationEditor::OnSettingsPressed(MouseEvent* event)
{
  mSettingsView->SetActive(!mSettingsView->GetActive());
}

//******************************************************************************
void AnimationEditor::CreateKeyFrameAtPlayHead(Property* property,
                                               HandleParam componentInstance,
                                               Any* newValue)
{
  // Do nothing if the property is not supported by the animation system
  if(!ValidPropertyTrack(property))
    return;

  RichAnimation* richAnimation = mEditorData->mRichAnimation;

  // Grab the component and object from the instance
  Component* component = componentInstance.Get<Component*>();
  Cog* object = component->GetOwner();

  // If we weren't given a value, query the object for its current value
  Any val;
  if(newValue != nullptr)
    val = *newValue;
  else
    val = property->GetValue(component);

  BoundType* componentTypeId = ZilchVirtualTypeId(component);

  // Attempt to find the property track
  TrackNode* propertyTrack = richAnimation->GetPropertyTrack(object,
                                              mAnimGraphObject, componentTypeId,
                                              property->Name);

  // If the property type has changed, notify the user and don't do anything
  if(property->PropertyType != propertyTrack->mPropertyTypeId)
  {
    String message = String::Format("The property '%s' has changed types.",
                                    propertyTrack->Path.c_str());
    DoNotifyWarning("Cannot create key frame", message);
    return;
  }

  // Update the value (or create a new key frame if it doesn't exist)
  float playHead = mScrubber->GetPlayHead();
  propertyTrack->UpdateOrCreateKeyAtTime(playHead, val);

  // Display this track
  mEditorData->AddToSelection(propertyTrack);
}

//******************************************************************************
Archetype* GetAnimationPreviewArchetype(Animation* animation)
{
  String previewArchetypeName;

  ContentItem* contentItem = animation->mContentItem;

  // Check both types of animations for the preview archetype
  if(GeneratedArchetype* genAchetype = contentItem->has(GeneratedArchetype))
    return ArchetypeManager::Find(genAchetype->mResourceId);

//   if(AnimationBuilder* builder = contentItem->has(AnimationBuilder))
//     previewArchetypeName = builder->mPreviewArchetype;
//   else if(RichAnimationBuilder* builder = contentItem->has(RichAnimationBuilder))
//     previewArchetypeName = builder->mPreviewArchetype;

  // Look up the archetype
  return ArchetypeManager::FindOrNull(previewArchetypeName);
}

//******************************************************************************
Cog* CreateAnimationPreview(Space* space, Animation* animation,
                            Archetype* archetype)
{
  // Look up the archetype if it wasn't given
  if(archetype == nullptr)
    archetype = GetAnimationPreviewArchetype(animation);

  if(archetype)
  {
    Cog* object = space->CreateAt(archetype->ResourceIdName, Vec3(0, 0, 0));
    
    if(AnimationGraph* animGraph = object->has(AnimationGraph))
    {
      AnimationNode* node = BuildBasic(animGraph, animation, 0, AnimationPlayMode::Loop);
      animGraph->SetActiveNode(node);
    }

    return object;
  }

  return nullptr;
}

} //namespace Zero
