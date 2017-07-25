///////////////////////////////////////////////////////////////////////////////
///
/// \file MainPropertyView.hpp
/// 
/// 
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class MetaSelection;
struct SelectionChangedEvent;
class PropertyView;
class SelectionHistory;
class PropertyInterface;
class PropertyToUndo;
class MultiPropertyInterface;
class OperationQueue;
class ResourcePropertyView;

//----------------------------------------------------------- Main Property View
class MainPropertyView : public Composite
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  MainPropertyView(Composite* parent, MetaSelection* selection,
                   OperationQueue* queue);
  ~MainPropertyView();

  /// Displays the given object in the property view. If it's a resource type,
  /// a preview of the resource will be created, and buttons specific
  /// to the resource will be added.
  void EditObject(HandleParam object, bool advanceHistory);

  /// Refreshes the property view.
  void Refresh();

  /// Resets the selected objects and selection history.
  void HardReset();

  SelectionHistory* GetHistory();
  PropertyView* GetPropertyView();

private:
  /// Event response.
  void OnSelectionChanged(SelectionChangedEvent* e);
  void OnSelectionFinal(SelectionChangedEvent* e);
  void OnArchetypeReload(ResourceEvent* e);

  /// History button events.
  void OnPreviousPressed(Event* e);
  void OnNextPressed(Event* e);
  void OnShowPressed(Event* e);
  void OnRecentPressed(Event* e);

  /// Resource button events.
  void OnExternalEdit(Event* e);
  void OnReloadContent(Event* e);

  /// Special logic for when a resource is selected. Will create the preview
  /// window as well as buttons specific to editing resources.
  void EditResource(HandleParam resource);
  void EditResources(MetaSelection* selection);

  /// Adds the buttons specific to resource editing.
  void AddResourceButtons();
  void DestroyResourceButtons();

  /// Creates a preview for the given resource.
  Handle PreviewResource(HandleParam object);

  /// Removes the preview area.
  void ClearPreview();

  /// Interfaces used for certain objects.
  PropertyToUndo* mUndoInterface;
  MultiPropertyInterface* mMultiInterface;

  /// The history of objects selected.
  SelectionHistory* mSelectionHistory;

  /// Used when the property grid is displaying different objects than 
  /// what's in the actual selection history (archetypes, content items, etc..).
  HandleOf<MetaSelection> mSpecialEdit;

  /// The primary operation queue of the editor.
  OperationQueue* mPrimaryOpQueue;

  /// Used when it doesn't make sense to have the property changes queued
  /// on the main editor's operation queue (like Archetypes).
  OperationQueue* mLocalOpQueue;

  /// The row of buttons on the top that include the selection history buttons
  /// and the buttons specific to resources when selected.
  Composite* mButtonRow;
  HandleOf<Widget> mExternalEditButton;
  HandleOf<Widget> mReloadResourceButton;

  /// The main property view to edit objects.
  PropertyView* mPropertyView;

  /// A splitter between the property view and the preview area
  Splitter* mSplitter;

  /// The area used to preview resources.
  Composite* mPreviewArea;
  HandleOf<Widget> mPreviewTile;
};

}//namespace Zero
