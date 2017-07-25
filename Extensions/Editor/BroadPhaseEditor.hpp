///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhaseEditor.hpp
/// 
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Forward declarations
class ComboBox;
class ListBox;
class CheckBox;
class IconButton;
class Editor;
class BroadPhaseTracker;

//----------------------------------------------------------- Broad Phase Editor
class BroadPhaseEditor : public Composite
{
public:
  /// Define the self type for connections.
  typedef BroadPhaseEditor ZilchSelf;

  BroadPhaseEditor(Editor* parent);

private:
  void RunTest(ObjectEvent* event);
  BroadPhaseTracker* CreateTracker();
  void AddBroadPhasesToTracker(BroadPhase::Type type, 
                               BroadPhaseTracker* tracker);
  void CreateGraphs(BroadPhase::Type type, BroadPhaseTracker* tracker);
  Widget* CreateRecordGraphWidget(BroadPhase::Type type, 
                          BroadPhaseTracker* tracker, BPStats::Type recordType);
  Widget* CreateStatsGraphWidget(BroadPhase::Type type, 
                                 BroadPhaseTracker* tracker);

  Composite* BuildPropertyGrid(StringParam label, BroadPhase::Type type, 
                               Composite* parent);

  /// Adds the selected broad phase to the active list
  void AddButtonPressed(BroadPhase::Type type);
  void DynamicAddButtonPressed(ObjectEvent* event);
  void StaticAddButtonPressed(ObjectEvent* event);

  /// Removes the selected broad phase from the active list
  void RemoveButtonPressed(BroadPhase::Type type);
  void DynamicRemoveButtonPressed(ObjectEvent* event);
  void StaticRemoveButtonPressed(ObjectEvent* event);

  void GameSpaceDestroyed(ObjectEvent* event);

  Editor* mEditor;

  HandleOf<GameSession> mActiveGame;

  ComboBox* mComboBoxes[BroadPhase::Size];
  ListBox* mListBoxes[BroadPhase::Size];
  TextCheckBox* mCheckBoxes[BPStats::Size];
  IconButton* mAddButtons[BroadPhase::Size];
  IconButton* mRemoveButtons[BroadPhase::Size];
  StringSource mBroadPhaseNames[BroadPhase::Size];
  StringSource mActiveBroadPhases[BroadPhase::Size];
  Array< HandleOf<Widget> > mActiveWindows;
};

}//namespace Zero
