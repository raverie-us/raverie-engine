///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationEditor.hpp
/// Declaration of AnimationEditor Composite
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations
class AnimationEditor;
class AnimationGraphEditor;
class AnimationScrubber;
class AnimationControls;
class AnimationDopeSheet;
class AnimationTrackView;
class AnimationSettings;
class AnimationSettingsView;
class PropertyEvent;
class MainPropertyView;
struct SelectionChangedEvent;
class AnimationEditorData;
class Spacer;
class Viewport;
class ContextMenuEvent;
class AnimationSelector;
class AnimationToolBox;

//------------------------------------------------------------- Animation Editor
DeclareEnum5(ErrorState,
             None,       // No Error
             NoObject,   // No object selected
             NoAnimGraph, // No time-line on the object
             ReadOnly,   // Animation is read only
             AnimationFromGeometry); // The animation was from a geometry file
                                     // and must first be converted to a
                                     // rich animation

//--------------------------------------------------------------- Key Frame Icon
/// This icon is added to the property grid for creating key frames.
class PropertyKeyIcon : public Composite
{
public:
  typedef PropertyKeyIcon ZilchSelf;
  PropertyKeyIcon(Composite* parent, HandleParam object,
                  Property* metaProperty, AnimationEditor* editor);

  void UpdateTransform() override;

  void OnMouseEnter(Event* e);
  void OnLeftClick(Event* e);
  void OnMouseExit(Event* e);

  bool mMouseOver;
  Element* mIcon;
  PropertyHandle mProperty;
  Handle mComponentHandle;
  AnimationEditor* mEditor;
};

//------------------------------------------------------------- Animation Editor
class AnimationEditor : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// We want the main viewport to get keyboard input for creating keys.
  AnimationEditor(Composite* parent);
  ~AnimationEditor();

  /// Updates the transform of each child object based on our client size.
  void UpdateTransform() override;

  /// We need to connect to events from the property view to know
  /// when a property is changed on an object.
  void SetPropertyView(MainPropertyView* view);

  /// Sets the animation being edited.
  void SetAnimation(Animation* animation);

  /// Sets the status text in the lower left corner.
  void SetStatusText(StringParam text, ByteColor color = Color::Gray);

  /// Returns the current state of the editor.
  ErrorState::Type GetErrorState();

  /// Forward focus to the background object.
  bool TakeFocusOverride() override;

  /// Updates the selected time line object to the play head of the scrubber.
  void UpdateToScrubber();

  /// Returns the selected AnimationGraph object.
  Cog* GetAnimationGraphObject();
  Cog* GetSelectedObject();

  /// Returns whether or not the given property on the given object
  /// has key frames.
  bool HasKeyFrames(HandleParam component, Property* metaProperty);

  AnimationScrubber* GetScrubber();
  AnimationGraphEditor* GetGraph();
  AnimationSettings* GetSettings();
  AnimationEditorData* GetEditorData();

private:
  friend class AnimationEditorComposite;
  friend class AnimationSelector;

  /// Updates the color and position of the negative area.
  void UpdateNegativeArea();

  /// We want to modify the animation of the selected object.
  void OnSelectionChanged(SelectionChangedEvent* event);

  /// When an object is selected, we must make sure it is valid to edit.
  /// This will update any error states as well as creating the proper animation
  /// info that the object has selected.
  void ObjectSelected(Cog* object);

  AnimationEditorData* GetEditorData(Cog* animGraphCog, Animation* animation);

  /// When we de-select an object, we want to stop displaying information
  /// about that animation, but we don't want to de-allocate and erase
  /// all state we had while editing it in case it was an accident.
  /// Hide and Show will help us retain state, but not display anything.
  void HideControls();
  void ShowControls();

  /// Calls ObjectSelected with the currently selected object in order
  /// to refresh error states when the object has been modified.
  void RefreshSelection();

  /// When the animation is modified, we want to send an event to tell
  /// the window tab we're under that we've been modified.
  void OnAnimationModified(Event* e);

  /// We only want to save the rich animation when the Save event is sent.
  void OnSave(Event* e);

  /// We need to know when a property changes for auto-key.
  void OnPropertyChanged(PropertyEvent* event);

  /// We want to update the state if the AnimationGraph component is
  /// added or removed.
  void OnComponentsChanged(PropertyEvent* event);

  /// We want to update the animation while scrubbing.
  void OnPlayHeadModified(Event* event);

  /// Updates the objects in the active animation to the given time.
  void UpdateToTime(float t);

  /// Sets the current error state.
  void SetErrorState(ErrorState::Type state);

  /// Before we draw, we want to flush all changes made to the rich animation
  /// to let the UI respond to modification events.
  void OnUpdate(UpdateEvent* e);

  /// Sends an event on the property view and main view port to update
  /// window highlights depending on whether or not auto key is enabled.
  void SendHighlightEvent(float dt);

  /// When the create key frame shortcut is pressed on the main view port,
  /// we want to create a key frame at the current play head.
  void OnKeyDownViewport(KeyboardEvent* e);

  /// We want the same functionality as the OnkeyDownViewport function,
  /// but if it's pressed when the animator has focus. We also want to 
  /// have the graph area focus on the selected curves when 'F' is pressed,
  /// but we don't want that to happen when it's pressed in the main view port.
  void OnKeyDown(KeyboardEvent* e);
  void OnKeyRepeated(KeyboardEvent* e);
  void UpdateArrowMovement(KeyboardEvent* e);

  /// When a property in the property grid is right clicked, we want
  /// to add a menu item to create a key frame if the property can be animated.
  void OnPropertyContextMenu(ContextMenuEvent* e);

  /// The callback for when the user clicks on the create key frame option.
  void OnCreateKeyFrame(Event* e);

  /// Mouse event response for the error text.
  void OnMouseEnterErrorText(MouseEvent* event);
  void OnMouseDownErrorText(MouseEvent* event);
  void OnMouseExitErrorText(MouseEvent* event);

  void OnMouseDownNegativeX(MouseEvent* event);

  /// When the editor opens the Add Resource dialog, we want events
  /// back to know when and if a resource was added so that we can
  /// edit and apply it to the currently selected object.
  void OpenAnimationAddWindow();
  void OnAnimationAdded(ResourceEvent* e);
  void OnAnimationAddCancel(Event* e);

  /// When the settings button is pressed on the controls view, we want
  /// to open the settings view.
  void OnSettingsPressed(MouseEvent* event);

public:
  /// Creates a key frame at the play head for the given property on the
  /// given instance. If newValue is NULL, it will query the component
  /// for the current value.
  void CreateKeyFrameAtPlayHead(Property* property,
                                HandleParam componentInstance,
                                Any* newValue = nullptr);

private:
  /// The current error state.
  ErrorState::Type mErrorState;

  /// The text object used to display the current error state.
  Label* mErrorStateText;
  HandleOf<ToolTip> mErrorToolTip;

  ScrollingGraph mGraphData;

  /// Settings for the editor.
  AnimationSettings mSettings;

  /// Controls.
  AnimationScrubber* mScrubber;
  AnimationGraphEditor* mGraph;
  AnimationDopeSheet* mDopeSheet;
  AnimationTrackView* mTrackView;
  AnimationSettingsView* mSettingsView;
  AnimationControls* mControls;
  AnimationSelector* mSelector;
  AnimationToolBox* mToolBox;

  /// The object being modified.
  CogId mSelectedObject;
  CogId mAnimGraphObject;

  OnionSkinning mOnionSkinning;

  /// We need to know about the property view to refresh it when we add
  /// the AnimationGraph component to an object.
  MainPropertyView* mPropertyView;
  HandleOf<Viewport> mMainViewport;

  /// The animation being edited.
  HandleOf<Animation> mAnimation;
  AnimationEditorData* mEditorData;

  /// Used to keep track of what property someone right clicked on
  /// so that if they click the "Create Key Frame" option, we still have
  /// the information.
  PropertyHandle mContextMenuProperty;
  Handle mContextMenuInstance;

  HashMap<ResourceId, AnimationEditorData*> mCachedAnimationData;
};

Archetype* GetAnimationPreviewArchetype(Animation* animation);

Cog* CreateAnimationPreview(Space* space, Animation* animation,
                            Archetype* previewArchetype = NULL);

} //namespace Zero
