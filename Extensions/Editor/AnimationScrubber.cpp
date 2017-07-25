///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationScrubber.cpp
/// Implementation of the AnimationScrubber Composite
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace AnimScrubberUi
{
const cstr cLocation = "EditorUi/AnimationEditor/Scrubber";
Tweakable(Vec4, BackgroundColor,        Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, AutoKeyBackgroundColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, KeyFrame,               Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, KeyFrameHover,          Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, KeyFrameSelected,       Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, KeyFrameSelectedHover,  Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, PlayHeadColor,          Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, GhostPlayHeadColor,     Vec4(1,1,1,1), cLocation);
}

namespace Events
{
  DefineEvent(PlayHeadModified);
}

//------------------------------------------------------------------ Scrub Graph
class ScrubberDrawer : public Widget
{
public:
  /// Constructor.
  ScrubberDrawer(AnimationScrubber* parent);

  /// WidgetCustomDraw interface.
  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  void DrawHashes(Array<StreamedVertex>& lines, ByteColor color, ScrollingGraph::range r, float hashHeight);

  /// Draw the vertical hash marks.
  void DrawHashMarks(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines);

  /// Draws the ghost play head line.
  void DrawGhostPlayHead(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines);

  /// Draws the play head line and the text.
  void DrawPlayHead(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines);
  
  /// The height of the hash marks.
  float mHashHeight;

  /// The font to render text.
  RenderFont* mFont;

  /// A pointer back to the scrubber to know how to draw it.
  AnimationScrubber* mScrubber;
};

//-------------------------------------------------------- Key Frame Manipulator
//******************************************************************************
class KeyFrameManipulator : public MouseManipulation
{
public:
  AnimationScrubber* mScrubber;
  HashMap<KeyFrameIcon*, float> mTimeOffsets;

  //****************************************************************************
  KeyFrameManipulator(Mouse* mouse, AnimationScrubber* scrubber)
    : MouseManipulation(mouse, scrubber)
  {
    mScrubber = scrubber;

    float timeAtMouse = GetTimeAtMouse(mouse);

    // Find the offset from he mouse of each selected key frame so that
    // we can apply that offset when the mouse moves
    forRange(KeyFrameIcon* keyFrame, mScrubber->mSelection.All())
    {
      float timeOffset = keyFrame->mTime - timeAtMouse;
      mTimeOffsets.Insert(keyFrame, timeOffset);
    }
  }
  
  //****************************************************************************
  float GetTimeAtMouse(Mouse* mouse)
  {
    float mousePosX = mScrubber->ToLocal(mouse->GetClientPosition()).x;
    return mScrubber->ToTime(mousePosX);
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* e) override
  {
    float timeAtMouse = GetTimeAtMouse(e->GetMouse());
    
    // Move each key frame
    forRange(KeyFrameIcon* keyFrame, mScrubber->mSelection.All())
    {
      float timeOffset = mTimeOffsets.FindValue(keyFrame, Math::cInfinite);
      ErrorIf(timeOffset == Math::cInfinite, "Selection changed while dragging.");

      keyFrame->MoveTo(timeAtMouse + timeOffset);
    }
  }

  //****************************************************************************
  void OnMouseUpdate(MouseEvent* e) override
  {
    mScrubber->MouseDragUpdate(e);
    OnMouseMove(e);
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* e) override
  {
    forRange(KeyFrameIcon* keyFrame, mScrubber->mSelection.All())
    {
      keyFrame->FinishMove();
    }
    this->Destroy();
  }
};

//-------------------------------------------------------------------- Key Frame
//******************************************************************************
KeyFrameIcon::KeyFrameIcon(AnimationScrubber* scrubber, float time)
  : Composite(scrubber)
{
  // Create the icon
  mIcon = CreateAttached<Element>(cKeyFrameImage);

  ConnectThisTo(mIcon, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(mIcon, Events::LeftClick, OnLeftClick);
  ConnectThisTo(mIcon, Events::LeftMouseDrag, OnLeftMouseDrag);
  ConnectThisTo(mIcon, Events::RightClick, OnRightClick);
  ConnectThisTo(mIcon, Events::RightMouseUp, OnRightMouseUp);

  mScrubber = scrubber;
  mTime = time;
}

//******************************************************************************
void KeyFrameIcon::OnMouseEnter(MouseEvent* e)
{
  MarkAsNeedsUpdate();
}

//******************************************************************************
void KeyFrameIcon::OnMouseExit(MouseEvent* e)
{
  MarkAsNeedsUpdate();
}

//******************************************************************************
void KeyFrameIcon::OnLeftMouseDown(MouseEvent* e)
{
  e->Handled = true;
}

//******************************************************************************
void KeyFrameIcon::OnLeftClick(MouseEvent* e)
{
  HashSet<KeyFrameIcon*>& selection = mScrubber->mSelection;

  if(e->CtrlPressed)
  {
    // If we're already in the selection, remove ourself
    if(selection.Contains(this))
      selection.Erase(this);
    // Otherwise just add ourself to the selection
    else
      selection.Insert(this);
  }
  else
  {
    // If it's just a single click, select only ourself
    selection.Clear();
    selection.Insert(this);
  }

  MarkAsNeedsUpdate();
  e->Handled = true;
}

//******************************************************************************
void KeyFrameIcon::OnLeftMouseDrag(MouseEvent* e)
{
  HashSet<KeyFrameIcon*>& selection = mScrubber->mSelection;

  // If control is pressed, we want to copy this key frame
  if(e->CtrlPressed)
  {
    KeyFrameIcon* newKeyIcon = Duplicate();
    selection.Clear();
    selection.Insert(newKeyIcon);

    new KeyFrameManipulator(e->GetMouse(), mScrubber);
  }
  else
  {
    // If we're not already in the selection and shift (multi-select) isn't
    // pressed, then we want to only drag this key frame
    if(!e->ShiftPressed && !selection.Contains(this))
      selection.Clear();

    selection.Insert(this);

    new KeyFrameManipulator(e->GetMouse(), mScrubber);
  }

  MarkAsNeedsUpdate();
  e->Handled = true;
}

//******************************************************************************
void KeyFrameIcon::OnRightClick(MouseEvent* e)
{
  // We want access to the RighMouseUp
  e->Handled = true;
}

//******************************************************************************
void KeyFrameIcon::OnRightMouseUp(MouseEvent* e)
{
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));
  e->Handled = true;

  if(!mScrubber->mSelection.Contains(this))
  {
    mScrubber->mSelection.Clear();
    mScrubber->mSelection.Insert(this);
  }

  // Create the delete button
  ConnectMenu(menu, "Delete", OnDelete);
}

//******************************************************************************
void KeyFrameIcon::OnDelete(ObjectEvent* e)
{
  mScrubber->DeleteSelectedKeys();
}

//******************************************************************************
void KeyFrameIcon::Delete()
{
  // We want to ignore all modification events as we're modifying it directly
  mScrubber->mIgnoreAnimationEvents = true;

  RichAnimation* richAnimation = mScrubber->mEditorData->mRichAnimation;

  // Delete all key frames that we represent from the rich animation
  forRange(KeyFrame* keyFrame, mKeyFrames.All())
  {
    keyFrame->Destroy();
  }

  // Remove ourself from our parent
  mScrubber->mKeyFrames.EraseEqualValues(this);

  // Destroy ourself
  Destroy();

  mScrubber->mIgnoreAnimationEvents = false;
}

//******************************************************************************
void KeyFrameIcon::MoveTo(float time)
{
  // Key frames cannot have a negative time
  time = Math::Max(time, 0.0f);

  // We want to ignore all modification events as we're modifying it directly
  mScrubber->mIgnoreAnimationEvents = true;

  RichAnimation* richAnimation = mScrubber->mEditorData->mRichAnimation;

  // Delete all key frames that we represent from the rich animation
  forRange(KeyFrame* keyFrame, mKeyFrames.All())
  {
    keyFrame->SetTime(time);
  }

  // Set the updated time
  mTime = time;

  // We want to be at the updated time in the array map
  mScrubber->ReInsert(this);

  mScrubber->MarkAsNeedsUpdate();

  mScrubber->mIgnoreAnimationEvents = false;
}

//******************************************************************************
void KeyFrameIcon::FinishMove()
{
  mScrubber->mIgnoreAnimationEvents = true;

  ScrollingGraph& graphData = *mScrubber->mGraphData;
  AnimationSettings* settings = mScrubber->mEditor->GetSettings();

  Vec2 graphPos(mTime, 0);
  Vec2 clientArea = mScrubber->GetSize();

  // Only snap if it's set in the settings object
  float time = graphPos.x;
  if(settings->mSnappingX)
    time = graphData.SnapGraphPos(graphPos, clientArea).x;

  // If we're moving them to a time that already has a key frame icon,
  // just add them to that icon and destroy ourself
  KeyFrameIcon* destinationIcon = mScrubber->mKeyFrames.FindValue(time, nullptr);

  if(destinationIcon)
  {
    mScrubber->mIgnoreAnimationEvents = true;

    // Delete all key frames that we represent from the rich animation
    forRange(KeyFrame* keyFrame, mKeyFrames.All())
    {
      // Set the time
      keyFrame->SetTime(time);

      // Add it to the destination icon (if it isn't ourself)
      if(destinationIcon != this)
      {
        ErrorIf(destinationIcon->mKeyFrames.Contains(keyFrame), "Key frame already added");
        destinationIcon->mKeyFrames.PushBack(keyFrame);
      }
    }

    // Clear our key frames as we've passed them off to the destination icon
    if(destinationIcon != this)
    {
      mKeyFrames.Clear();

      // Destroy ourself
      OnDelete(nullptr);
    }

    mScrubber->mIgnoreAnimationEvents = false;
  }
  else
  {
    // Snap the key to the nearest frame
    MoveTo(time);
  }

  // Mark the animation as modified
  //mScrubber->mEditorData->Modified();

  mScrubber->SetPlayHead(mScrubber->GetPlayHead(), true);
}

//******************************************************************************
void KeyFrameIcon::Hide()
{
  mIcon->SetVisible(false);
  mIcon->SetInteractive(false);
}

//******************************************************************************
void KeyFrameIcon::Show()
{
  mIcon->SetVisible(true);
  mIcon->SetInteractive(true);
}

//******************************************************************************
bool KeyFrameIcon::AttemptRemove(KeyFrame* keyFrame)
{
  size_t index = mKeyFrames.FindIndex(keyFrame);
  if(index != Array<KeyFrame*>::InvalidIndex)
  {
    mKeyFrames.EraseAt(index);
    MarkAsNeedsUpdate();
    return true;
  }

  return false;
}

//******************************************************************************
KeyFrameIcon* KeyFrameIcon::Duplicate()
{
  // We want to create the new key frame just slightly to the right of the
  // current so they're not collapsed on the same
  const float cTimeOffset = 0.01f;
  float newTime = mTime + cTimeOffset;

  // Create 
  KeyFrameIcon* duplicate = new KeyFrameIcon(mScrubber, newTime);
  mScrubber->ReInsert(duplicate);

  duplicate->mKeyFrames.Reserve(mKeyFrames.Size());

  // We want to ignore all modification events as we're modifying it directly
  mScrubber->mIgnoreAnimationEvents = true;

  // Duplicate and add each key frame of the rich animation
  forRange(KeyFrame* keyFrame, mKeyFrames.All())
  {
    KeyFrame* duplicateKeyFrame = keyFrame->Duplicate(newTime);
    duplicate->mKeyFrames.PushBack(duplicateKeyFrame);
  }

  mScrubber->mIgnoreAnimationEvents = false;

  return duplicate;
}

//******************************************************************************
void KeyFrameIcon::UpdateTransform()
{
  // Fill the icon
  mIcon->SetSize(mSize);

  if(mScrubber->mSelection.Contains(this))
  {
    if(mIcon->IsMouseOver())
      mIcon->SetColor(AnimScrubberUi::KeyFrameSelectedHover);
    else
      mIcon->SetColor(AnimScrubberUi::KeyFrameSelected);
  }
  else
  {
    if(mIcon->IsMouseOver())
      mIcon->SetColor(AnimScrubberUi::KeyFrameHover);
    else
      mIcon->SetColor(AnimScrubberUi::KeyFrame);
  }

  Composite::UpdateTransform();
}

//******************************************************************************
void KeyFrameIcon::OnDestroy()
{
  // Remove ourself from the selection
  mScrubber->mSelection.Erase(this);

  Composite::OnDestroy();
}

//--------------------------------------------------------- Scrubber Manipulator
//******************************************************************************
DeclareEnum2(ScrubberMode, Scrub, Scroll);

class ScrubberManipulator : public MouseManipulation
{
public:
  AnimationScrubber* mScrubber;
  ScrubberMode::Type mMode;
  float mStartPos;
  float mGraphStart;

  //****************************************************************************
  ScrubberManipulator(Mouse* mouse, AnimationScrubber* scrubber, 
                      ScrubberMode::Type mode, float startPos = 0.0f) 
    : MouseManipulation(mouse, scrubber)
  {
    mScrubber = scrubber;
    mMode = mode;
    mStartPos = startPos;

    ScrollingGraph& graph = *mScrubber->mGraphData;
    mGraphStart = graph.Translation.x;

    UpdateScrubberPosition(mouse);
  }

  //****************************************************************************
  void UpdateScrubberPosition(Mouse* mouse)
  {
    // Get the local position of the mouse
    Vec2 pixelPos = mScrubber->ToLocal(mouse->GetClientPosition());

    // If we're scrubbing, convert and set the play head
    if(mMode == ScrubberMode::Scrub)
    {
      float playHead = mScrubber->ToTime(pixelPos.x);
      mScrubber->SetPlayHead(playHead);
    }
    // If we're sliding, apply the offset and set the target slide position
    else
    {
      ScrollingGraph& graph = *mScrubber->mGraphData;

      float pixelOffset = pixelPos.x - mStartPos;
      graph.ScrollPixels(Vec2(mouse->mRawMovement.x, 0));
      mStartPos = pixelPos.x;
      mScrubber->MarkAsNeedsUpdate();
    }
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* e) override
  {
    UpdateScrubberPosition(e->GetMouse());
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* event) override
  {
    Finish();
  }

  //****************************************************************************
  void OnRightMouseUp(MouseEvent* event) override
  {
    Finish();
  }

  //****************************************************************************
  void OnMiddleMouseUp(MouseEvent* event) override
  {
    Finish();
  }

  //****************************************************************************
  void Finish()
  {
    if(mMode == ScrubberMode::Scrub)
    {
      float playHead = mScrubber->GetPlayHead();
      Vec2 clientArea = mScrubber->GetSize();

      ScrollingGraph& graph = *mScrubber->mGraphData;

      // Only snap the position if enabled in the settings
      AnimationSettings* settings = mScrubber->mEditor->GetSettings();
      if(settings->mSnappingX)
        playHead = graph.SnapGraphPos(Vec2(playHead, 0), clientArea).x;

      mScrubber->SetPlayHead(playHead);
    }
    this->Destroy();
    
    mScrubber->mEditor->TakeFocus();
  }
};

//----------------------------------------------------------- Scrubber Selection
//******************************************************************************
class ScrubberSelection : public MouseManipulation
{
public:
  AnimationScrubber* mScrubber;
  Vec2 mStart;

  //****************************************************************************
  ScrubberSelection(Mouse* mouse, AnimationScrubber* scrubber)
    : MouseManipulation(mouse, scrubber)
  {
    mScrubber = scrubber;
    mStart = scrubber->ToLocal(mouse->GetClientPosition());
  }

  //****************************************************************************
  Rect GetSelectionRect(MouseEvent* e)
  {
    // Get the local position of the mouse
    Vec2 curr = mScrubber->ToLocal(e->Position);

    Vec2 min = Math::Min(curr, mStart);
    Vec2 max = Math::Max(curr, mStart);

    min.x = Math::Max(min.x, mScrubber->mNegativeArea->GetSize().x);
    max.x = Math::Min(max.x, mScrubber->mSize.x);

    min.y = 0.0f;
    max.y = mScrubber->mSize.y * 0.7f;

    // This should be enabled later for when event tracks are added
    //if(min.y < mScrubber->mSize.y * 0.7f)
    //  min.y = 0.0f;
    //else
    //  min.y = mScrubber->mSize.y * 0.7f;
    //
    //if(max.y > mScrubber->mSize.y * 0.7f)
    //  max.y = mScrubber->mSize.y;
    //else
    //  max.y = mScrubber->mSize.y * 0.7f;
    
    return Rect::MinAndMax(min, max);
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* e) override
  {
    Rect selection = GetSelectionRect(e);
    PlaceWithRect(selection, mScrubber->mSelectBox);

    // Make it visible now that we've moved
    mScrubber->mSelectBox->SetVisible(true);
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* e) override
  {
    Rect selection = GetSelectionRect(e);

    // We're not adding to the selection if ctrl is pressed, so clear it
    if(!e->CtrlPressed)
      mScrubber->mSelection.Clear();

    // Add all overlapping key frames to the selection
    forRange(KeyFrameIcon* keyFrame, mScrubber->mKeyFrames.AllValues())
    {
      Rect keyFrameRect = keyFrame->GetRectInParent();

      if(selection.Overlap(keyFrameRect))
      {
        mScrubber->mSelection.Insert(keyFrame);
        keyFrame->MarkAsNeedsUpdate();
      }
    }

    mScrubber->mSelectBox->SetVisible(false);
    this->Destroy();
  }
};

//----------------------------------------------------------- Animation Scrubber
//******************************************************************************
AnimationScrubber::AnimationScrubber(Composite* parent, AnimationEditor* editor,
                                     ScrollingGraph* graphData)
  : Composite(parent)
{
  mEditor = editor;
  mEditorData = nullptr;
  mGraphData = graphData;
  mIgnoreAnimationEvents = false;
  SetClipping(true);

  mEnabled = false;

  mShowGhostPlayHead = false;
  mGhostPlayHead = 0.0f;

  mPlayHead = 0.0f;
  mTargetSlidePosition = mGraphData->Translation.x;

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  // Mouse Events
  ConnectThisTo(this, Events::LeftClick, OnLeftMouseClick);
  ConnectThisTo(this, Events::LeftMouseDrag, OnLeftMouseDrag);
  ConnectThisTo(this, Events::RightClick, OnRightClick);
  ConnectThisTo(this, Events::MiddleMouseDown, OnMiddleMouseDown);
  ConnectThisTo(this, Events::MouseScroll, OnMouseScroll);

  // Create the background element
  mBackground = CreateAttached<Element>(cBackgroundElement);

  // Create the scrub drawer
  mDrawer = new ScrubberDrawer(this);
  mDrawer->SetClipping(false);

  mNegativeArea = CreateAttached<Element>(cWhiteSquare);

  mSelectBox = CreateAttached<Element>(cDragBox);
  mSelectBox->SetVisible(false);
}

//******************************************************************************
void AnimationScrubber::SetAnimationEditorData(AnimationEditorData* editorData)
{
  mEditorData = editorData;

  // Connect to events so we know when the animation has been modified
  RichAnimation* richAnimation = mEditorData->mRichAnimation;

  // Disconnect from old events
  DisconnectAll(richAnimation, this);
  DisconnectAll(mEditorData, this);

  // Connect again
  ConnectThisTo(richAnimation, Events::TrackDeleted, OnTrackDeleted);
  ConnectThisTo(richAnimation, Events::KeyFrameAdded, OnKeyFrameAdded);
  ConnectThisTo(richAnimation, Events::KeyFrameModified, OnKeyFrameModified);
  ConnectThisTo(richAnimation, Events::KeyFrameDeleted, OnKeyFrameDeleted);
  ConnectThisTo(mEditorData, Events::TrackSelectionModified, OnSelectionModified);

  RebuildKeyFrames();
}

//******************************************************************************
float AnimationScrubber::GetPlayHead()
{
  return mPlayHead;
}

//******************************************************************************
void AnimationScrubber::SetPlayHead(float t, bool sendsEvent)
{
  // Don't let it go negative
  mPlayHead = Math::Max(0.0f, t);

  // Send an event
  if(sendsEvent)
  {
    Event e;
    GetDispatcherObject()->Dispatch(Events::PlayHeadModified, &e);
  }

  // We need to re-draw
  mDrawer->MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationScrubber::DestroyKeyFrames()
{
  // Destroy all key frames
  forRange(KeyFrameIcon* keyFrame, mKeyFrames.AllValues())
  {
    keyFrame->Destroy();
  }
  mKeyFrames.Clear();
  mSelection.Clear();
}

//******************************************************************************
void AnimationScrubber::FrameSelectedKeyFrames(KeyFrameSelection& selection)
{

}

//******************************************************************************
void AnimationScrubber::Hide()
{
  mEnabled = false;
  forRange(KeyFrameIcon* keyFrame, mKeyFrames.AllValues())
  {
    keyFrame->Hide();
  }
}

//******************************************************************************
void AnimationScrubber::Show()
{
  mEnabled = true;
  forRange(KeyFrameIcon* keyFrame, mKeyFrames.AllValues())
  {
    keyFrame->Show();
  }
}

//******************************************************************************
void AnimationScrubber::DeleteSelectedKeys()
{
  mIgnoreAnimationEvents = true;
  
  // Move to a temporary array as they will be removed from the
  // selection when we call delete
  Array<KeyFrameIcon*> keysToDestroy;
  keysToDestroy.Reserve(mSelection.Size());
  forRange(KeyFrameIcon* key, mSelection.All())
  {
    keysToDestroy.PushBack(key);
  }
  forRange(KeyFrameIcon* key, keysToDestroy.All())
  {
    key->Delete();
  }
  mSelection.Clear();
  mIgnoreAnimationEvents = false;
}

//******************************************************************************
void AnimationScrubber::AddKeyFrame(KeyFrame* keyFrame)
{
  // Get the time the key frame is at
  float keyTime = keyFrame->GetTime();

  // Attempt to find an icon already 
  KeyFrameIcon* keyIcon = mKeyFrames.FindValue(keyTime, nullptr);

  // If it doesn't exist, create a new one
  if(keyIcon == nullptr)
  {
    keyIcon = new KeyFrameIcon(this, keyTime);
    mKeyFrames.Insert(keyTime, keyIcon);
  }

  // Add the new key frame
  ErrorIf(keyIcon->mKeyFrames.Contains(keyFrame), "Key frame already added");
  keyIcon->mKeyFrames.PushBack(keyFrame);

  // We need to update the transform of each key frame
  MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationScrubber::RebuildKeyFrames()
{
  // Clear any existing key frames
  DestroyKeyFrames();

  // We only want property tracks
  forRange(TrackNode* track, mEditorData->mVisiblePropertyTracks.All())
  {
    forRange(KeyFrame* keyFrame, track->mKeyFrames.AllValues())
    {
      AddKeyFrame(keyFrame);
    }
  }
}

//******************************************************************************
void AnimationScrubber::OnSelectionModified(Event* event)
{
  RebuildKeyFrames();
}

//******************************************************************************
void AnimationScrubber::OnTrackDeleted(TrackEvent* event)
{
  // TODO: Only rebuild key frames if we were displaying a key frame from
  // the track that was deleted
  RebuildKeyFrames();
}

//******************************************************************************
void AnimationScrubber::OnKeyFrameAdded(KeyFrameEvent* event)
{
  if(mIgnoreAnimationEvents)
    return;

  KeyFrame* keyFrame = event->mKeyFrame;
  TrackNode* track = keyFrame->GetParentTrack();

  // Only display the key frame if it's in the visible track list
  if(mEditorData->mVisiblePropertyTracks.FindValue(track, nullptr))
    AddKeyFrame(keyFrame);
}

//******************************************************************************
void AnimationScrubber::OnKeyFrameModified(KeyFrameEvent* event)
{
  if(mIgnoreAnimationEvents)
    return;

  KeyFrame* movedKeyFrame = event->mKeyFrame;

  forRange(KeyFrameIcon* keyFrameIcon, mKeyFrames.AllValues())
  {
    // If this key icon Contains the moved key frame
    if(keyFrameIcon->mKeyFrames.Contains(movedKeyFrame))
    {
      float destinationTime = movedKeyFrame->GetTime();

      // If the time of the key frame didn't change, there is no need to
      // do anything as it doesn't change what data we're displaying
      if(keyFrameIcon->mTime != destinationTime)
      {
        // If there's another icon at the destination, we can move the key frame
        // to that icon instead of creating a new one
        KeyFrameIcon* destination = mKeyFrames.FindValue(destinationTime, nullptr);

        if(destination)
        {
          // Remove it from the current icon
          keyFrameIcon->mKeyFrames.EraseValueError(movedKeyFrame);

          // Add it to the destination icon
          ErrorIf(destination->mKeyFrames.Contains(movedKeyFrame), "Key frame already added");
          destination->mKeyFrames.PushBack(movedKeyFrame);

          // If the icon it came from doesn't have any more key frames, delete it
          if(keyFrameIcon->mKeyFrames.Empty())
          {
            mKeyFrames.EraseEqualValues(keyFrameIcon);
            keyFrameIcon->Destroy();
          }
        }
        // There is no icon at the destination, so we must either create
        // a new one, or move the current one
        else
        {
          // If the key frame is the only one in the icon, we can
          // just move the key icon
          if(keyFrameIcon->mKeyFrames.Size() == 1)
          {
            // Just set the time
            keyFrameIcon->mTime = destinationTime;
            ReInsert(keyFrameIcon);
          }
          // If there's other key frames in this icon, We need to 
          // create a new icon
          else
          {
            // Remove it from current
            keyFrameIcon->mKeyFrames.EraseValueError(movedKeyFrame);

            // Create a new one
            KeyFrameIcon* newIcon = new KeyFrameIcon(this, destinationTime);
            newIcon->mKeyFrames.PushBack(movedKeyFrame);
            mKeyFrames.Insert(destinationTime, newIcon);

            UpdateTransform();
          }
        }
      }

      MarkAsNeedsUpdate();
      return;
    }
  }
}

//******************************************************************************
void AnimationScrubber::OnKeyFrameDeleted(KeyFrameEvent* event)
{
  if(mIgnoreAnimationEvents)
    return;

  // Remove the key frame from each key icon if it exists
  forRange(KeyFrameIcon* keyIcon, mKeyFrames.AllValues())
  {
    if(keyIcon->AttemptRemove(event->mKeyFrame))
    {

      // If it doesn't have any more key frames, delete it
      if(keyIcon->mKeyFrames.Empty())
      {
        // Erase the key icon
        mKeyFrames.EraseEqualValues(keyIcon);

        // Destroy it
        keyIcon->Destroy();
      }
      break;
    }
  }
}

//******************************************************************************
void AnimationScrubber::OnKeyDown(KeyboardEvent* e)
{
  if(e->Key == Keys::Delete)
    DeleteSelectedKeys();
}

//******************************************************************************
float AnimationScrubber::ToTime(float localPos)
{
  return mGraphData->ToGraphSpace(Vec2(localPos - Pixels(30), 0)).x;
}

//******************************************************************************
float AnimationScrubber::ToPixels(float worldPos)
{
  return mGraphData->ToPixels(Vec2(worldPos, 0)).x + Pixels(30);
}

//******************************************************************************
void AnimationScrubber::OnLeftMouseClick(MouseEvent* event)
{
  if(!mEnabled || event->Handled)
    return;

  if(event->ShiftPressed)
  {
    mSelection.Clear();
  }
  else
  {
    Vec2 pixelPos = ToLocal(event->Position);
    float playHead = ToTime(pixelPos.x);
    SetPlayHead(playHead);
  }
}

//******************************************************************************
void AnimationScrubber::OnLeftMouseDrag(MouseEvent* event)
{
  if(!mEnabled || event->Handled)
    return;

  if(event->ShiftPressed)
    new ScrubberSelection(event->GetMouse(), this);
  else
    new ScrubberManipulator(event->GetMouse(), this, ScrubberMode::Scrub);
}

//******************************************************************************
void AnimationScrubber::OnRightClick(MouseEvent* event)
{
  if(event->Handled)
    return;

  mSelection.Clear();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationScrubber::OnMiddleMouseDown(MouseEvent* event)
{
  if(!mEnabled)
    return;

  float localMouse = ToLocal(event->Position).x;
  float startPos = -mGraphData->Translation.x + localMouse;

  new ScrubberManipulator(event->GetMouse(), this, ScrubberMode::Scroll, startPos);
}

//******************************************************************************
void AnimationScrubber::OnMouseScroll(MouseEvent* e)
{
  if(!mEnabled)
    return;

  Vec2 zoomPos = this->ToLocal(e->Position) - Pixels(30, 0);
  float zoom = e->Scroll.y * 0.02f * mGraphData->GetZoom().x;
  zoomPos.y = GetSize().y - zoomPos.y;
  mGraphData->ZoomAtPosition(zoomPos, Vec2(zoom, 0));

  MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationScrubber::MouseDragUpdate(MouseEvent* e)
{
  Vec2 local = ToLocal(e->Position);

  const float cDragSize = Pixels(25);
  const float maxSpeed = Pixels(6);
  float scroll = 0.0f;

  if(local.x < cDragSize)
  {
    float percent = 1.0f - (local.x / cDragSize);
    scroll = percent * maxSpeed;
  }
  else if(local.x > mSize.x - cDragSize)
  {
    float percent = 1.0f - (mSize.x - local.x) / cDragSize;
    scroll = -percent * maxSpeed;
  }

  scroll = Math::Clamp(scroll, -maxSpeed, maxSpeed);
  mGraphData->ScrollPixels(Vec2(scroll, 0));
  mGraphData->Translation.x = Math::Min(0.0f, mGraphData->Translation.x);
}

//******************************************************************************
void AnimationScrubber::UpdateTransform()
{
  // Fill the background
  mBackground->SetSize(mSize);
  if(mEditor->GetSettings()->mAutoKey)
    mBackground->SetColor(AnimScrubberUi::AutoKeyBackgroundColor);
  else
    mBackground->SetColor(AnimScrubberUi::BackgroundColor);

  // Fill the drawer
  mDrawer->SetSize(mSize);

  // Update the key frames
  forRange(KeyFrameIcon* key, mKeyFrames.AllValues())
  {
    float localX = ToPixels(key->mTime);
    float size = Pixels(10);
    key->SetTranslation(SnapToPixels(Vec3(localX - (size * 0.5f), Pixels(2), 0)));
    key->SetSize(Vec2(size, size));
  }

  // Update the base class
  Composite::UpdateTransform();
}

//******************************************************************************
void AnimationScrubber::ReInsert(KeyFrameIcon* icon)
{
  mKeyFrames.EraseEqualValues(icon);
  mKeyFrames.Insert(icon->mTime, icon);
}

//------------------------------------------------------------------ Scrub Graph
//******************************************************************************
ScrubberDrawer::ScrubberDrawer(AnimationScrubber* parent) 
  : Widget(parent)
{
  mHashHeight = Pixels(20);
  mFont = FontManager::GetInstance()->GetRenderFont(cTextFont, 11, 0);
  mScrubber = parent;
}

//******************************************************************************
void ScrubberDrawer::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  Array<StreamedVertex> lines;

  DrawHashMarks(viewBlock, frameBlock, clipRect, lines);
  if(mScrubber->mEnabled)
  {
    DrawGhostPlayHead(viewBlock, frameBlock, clipRect, lines);
    DrawPlayHead(viewBlock, frameBlock, clipRect, lines);
  }

  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);
}

//******************************************************************************
void ScrubberDrawer::DrawHashes(Array<StreamedVertex>& lines, ByteColor color, ScrollingGraph::range r, float hashHeight)
{
  forRange(ScrollingGraph::HashMark entry, r)
  {
    if(entry.Position < 0.0f)
      continue;
    Vec3 start(mScrubber->ToPixels(entry.Position), 0, 0);
    Vec3 end = start + Vec3(0, hashHeight, 0);

    // Draw the line
    lines.PushBack(StreamedVertex(SnapToPixels(start), Vec2(0, 0), ToFloatColor(color)));
    lines.PushBack(StreamedVertex(SnapToPixels(end), Vec2(0, 0), ToFloatColor(color)));
  }
}

//******************************************************************************
void ScrubberDrawer::DrawHashMarks(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines)
{
  ScrollingGraph& graphData = *mScrubber->mGraphData;

  // A horizontal line across the top
  Vec4 color = ToFloatColor(AnimationConstants::cHashColor);
  lines.PushBack(StreamedVertex(Vec3(0,0,0), Vec2(0, 0), color));
  lines.PushBack(StreamedVertex(Vec3(mSize.x,0,0), Vec2(0, 0), color));

  DrawHashes(lines, AnimationConstants::cHashColor, graphData.GetWidthHashes(mSize.x), mHashHeight);
  DrawHashes(lines, AnimationConstants::cHashColor, graphData.GetWidthHashes(mSize.x, true), mHashHeight * 0.5f);

  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, mFont->mTexture);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, ToFloatColor(AnimationConstants::cHashColor));

  forRange(ScrollingGraph::HashMark entry, graphData.GetWidthHashes(mSize.x))
  {
    if(entry.Position < 0.0f)
      continue;
    Vec2 start(mScrubber->ToPixels(entry.Position), 0);
    String label = entry.Label;
    Vec2 textSize = mFont->MeasureText(label, 1.0f);
    Vec2 labelPos = start + Vec2(-textSize.x * 0.5f, mHashHeight * 1.35f);
    ProcessTextRange(fontProcessor, mFont, label, labelPos, TextAlign::Left, Vec2(1, 1), textSize);
  }
}

//******************************************************************************
void ScrubberDrawer::DrawGhostPlayHead(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines)
{
  if(!mScrubber->mShowGhostPlayHead)
    return;

  float localX = mScrubber->ToPixels(mScrubber->mGhostPlayHead);

  Vec4 color = AnimScrubberUi::GhostPlayHeadColor;
  lines.PushBack(StreamedVertex(SnapToPixels(Vec3(localX,0,0)), Vec2(0, 0), color));
  lines.PushBack(StreamedVertex(SnapToPixels(Vec3(localX, mScrubber->mSize.y, 0)), Vec2(0, 0), color));
}

//******************************************************************************
void ScrubberDrawer::DrawPlayHead(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines)
{
  float playHead = mScrubber->GetPlayHead();
  float localX = mScrubber->ToPixels(playHead);

  Vec4 color = AnimScrubberUi::PlayHeadColor;
  lines.PushBack(StreamedVertex(SnapToPixels(Vec3(localX, 0, 0)), Vec2(0, 0), color));
  lines.PushBack(StreamedVertex(SnapToPixels(Vec3(localX, mHashHeight * 2.15f, 0)), Vec2(0, 0), color));

  AnimationSettings* settings = mScrubber->mEditor->GetSettings();
  ScrollingGraph& graphData = *mScrubber->mGraphData;
  
  String text;
  if(settings->mTimeDisplay == TimeDisplay::Frames)
  {
    String format;
    if(settings->mSnappingX)
      format = graphData.GetWidthFormat(mScrubber->GetSize().x);
    else
      format = "%.2f";
    text = String::Format(format.c_str(), playHead * settings->mEditFps);
  }
  else
  {
    const float cSecondsPerMinute = 60.0f;
    const float cSecondsPerHour = cSecondsPerMinute * 60.0f;

    // 'playHead' is in seconds
    uint milliseconds = (uint)((playHead - Math::Truncate(playHead) + 0.000001f) * 100.0f);
    uint seconds = (uint)(Math::FMod(playHead, cSecondsPerMinute));
    uint minutes = (uint)(playHead / cSecondsPerMinute);
    uint hours = (uint)(playHead / (cSecondsPerHour));

    text = String::Format("%.2i:%.2i:%.2i", minutes, seconds, milliseconds);
  }

  Vec2 size = mFont->MeasureText(text, 1.0f);
  Vec2 textPos(localX - (size.x * 0.5f), mHashHeight * 2.15f);

  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, mFont->mTexture);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, ToFloatColor(AnimationConstants::cHashColor));
  ProcessTextRange(fontProcessor, mFont, text, SnapToPixels(textPos), TextAlign::Left, Vec2(1, 1), size);
}

}//namespace Zero
