///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhaseEditor.cpp
/// 
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------------- Record Sampler
class RecordSampler : public DataSampler
{
public:
  RecordSampler(StringParam name, Profile::Record* record)
  {
    mName = name;
    mRecord = record;
  }

  virtual void Setup(RangeData& data, EntryLabel& label)
  {
    label.Name = mName;
  }

  virtual float Sample()
  {
    mRecord->Update();
    float timeInS = Profile::ProfileSystem::Instance->GetTimeInSeconds((Profile::ProfileTime)mRecord->SmoothAverage());

    // Convert to milliseconds
    float ms = timeInS * 1000.0f;

    return ms;
  }

  String mName;
  Profile::Record* mRecord;
};

//----------------------------------------------------------- Statistics Sampler
class StatisticsSampler : public DataSampler
{
public:
  StatisticsSampler(StringParam name, Statistics* stats)
  {
    mName = name;
    mStats = stats;
  }

  virtual void Setup(RangeData& data, EntryLabel& label)
  {
    label.Name = mName;
  }

  virtual float Sample()
  {
    if(mStats->mPossibleCollisionsReturned == 0)
      return 0.0f;
    float percentageCorrect = float(mStats->mActualCollisions) / 
                              float(mStats->mPossibleCollisionsReturned);

    return 1.0f - percentageCorrect;
  }

  String mName;
  Statistics* mStats;
};

//------------------------------------------------------------------ Cos Sampler
class CosSampler : public DataSampler
{
public:
  virtual void Setup(RangeData& data, EntryLabel& label)
  {
    label.Name = "Cos";
  }

  float p;
  CosSampler()
  {
    p = 0.0f;
  }
  virtual float Sample()
  {
    p += 0.1f;
    return Math::Cos(p) * 0.5f + 0.5f;
  }
};

//----------------------------------------------------------- Broad Phase Editor
BroadPhaseEditor::BroadPhaseEditor(Editor* parent) 
  : Composite(parent)
{
  mEditor = parent;

  SetLayout(CreateStackLayout());

  // A composite to encompass both the dynamic and static add/remove boxes
  Composite* dynamicStatic = new Composite(this);
  dynamicStatic->SetLayout(CreateRowLayout());
  dynamicStatic->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
  {
    // Create the dynamic add/remove box
    BuildPropertyGrid("Dynamic", BroadPhase::Dynamic, dynamicStatic);

    // Connect to the add button
    IconButton* addButton = mAddButtons[BroadPhase::Dynamic];
    ConnectThisTo(addButton, Events::ButtonPressed, DynamicAddButtonPressed);
    IconButton* removeButton = mRemoveButtons[BroadPhase::Dynamic];
    ConnectThisTo(removeButton, Events::ButtonPressed, DynamicRemoveButtonPressed);

    // Add a spacer in between
    Spacer* spacer = new Spacer(dynamicStatic);
    spacer->SetSize(Pixels(1, 1));

    // Create the static add/remove box
    BuildPropertyGrid("Static", BroadPhase::Static, dynamicStatic);

    // Connect to the add button
    addButton = mAddButtons[BroadPhase::Static];
    ConnectThisTo(addButton, Events::ButtonPressed, StaticAddButtonPressed);
    removeButton = mRemoveButtons[BroadPhase::Static];
    ConnectThisTo(removeButton, Events::ButtonPressed, StaticRemoveButtonPressed);
  }
  
  for(uint i = 0; i < BPStats::Size; ++i)
  {
    mCheckBoxes[i] = new TextCheckBox(this);
    mCheckBoxes[i]->SetText(Zero::BuildString("Track ", BPStats::Names[i]));
  }

  TextButton* button1 = new TextButton(this);
  button1->SetText("Run Test");
  button1->SetToolTip("Run the test");
  button1->SetSize(Pixels(90, 24));
  ConnectThisTo(button1, Events::ButtonPressed, RunTest);
}

void BroadPhaseEditor::RunTest(ObjectEvent* event)
{
  // Check to make sure a dynamic broad phase is selected
  if(mActiveBroadPhases[BroadPhase::Dynamic].Empty())
  {
    DoNotifyWarning("Cannot Run Test", "You must add a Dynamic Broad Phase.");
    return;
  }

  // Check to make sure a static broad phase is selected
  if(mActiveBroadPhases[BroadPhase::Static].Empty())
  {
    DoNotifyWarning("Cannot Run Test", "You must add a Static Broad Phase.");
    return;
  }

  //stop the old game if it was running (and close the old ui)
  GameSession* oldGame = mActiveGame;
  if(oldGame != NULL)
  {
    mEditor->StopGame();
    GameSpaceDestroyed(NULL);
  }

  // Run the game
  GameSession* game = mEditor->PlayGame(PlayGameOptions::MultipleInstances);
  mActiveGame = game;

  Space* gameSpace = game->GetAllSpaces().Front();

  ConnectThisTo(gameSpace, Events::SpaceDestroyed, GameSpaceDestroyed);

  // Create the tracker
  BroadPhaseTracker* tracker = CreateTracker();

  // Replace and delete the old broad phase
  PhysicsSpace* physicsSpace = gameSpace->has(PhysicsSpace);
  BroadPhasePackage* oldBroadPhase = physicsSpace->ReplaceBroadPhase(tracker);
  delete oldBroadPhase;

  CreateGraphs(BroadPhase::Dynamic, tracker);
  //CreateGraphs(BroadPhase::Static, tracker);
}

BroadPhaseTracker* BroadPhaseEditor::CreateTracker()
{
  // Allocate the tracker
  BroadPhaseTracker* tracker = new BroadPhaseTracker();
  
  // Allocate and add all the active broad phases
  AddBroadPhasesToTracker(BroadPhase::Dynamic, tracker);
  AddBroadPhasesToTracker(BroadPhase::Static, tracker);

  return tracker;
}

void BroadPhaseEditor::AddBroadPhasesToTracker(BroadPhase::Type type, 
                                               BroadPhaseTracker* tracker)
{
  for(uint i = 0; i < mActiveBroadPhases[type].Size(); ++i)
  {
    // We need to add the "BroadPhase" back to the end of the name
    String name = mActiveBroadPhases[type][i];
    name = BuildString(name, "BroadPhase");

    // Create the broad phase
    IBroadPhase* broadPhase = Z::gBroadPhaseLibrary->CreateBroadPhase(name);

    // Add it to the tracker
    tracker->AddBroadPhase(type, broadPhase);
  }
}

void BroadPhaseEditor::CreateGraphs(BroadPhase::Type type, 
                                    BroadPhaseTracker* tracker)
{
  // Create the graph for percentage correct
  Widget* statsGraph = CreateStatsGraphWidget(type, tracker);
  Window* window = mEditor->AddManagedWidget(statsGraph, DockArea::Right, true);
  
  //Remove for now
  mActiveWindows.PushBack(window);

  // Create the record graphs
  for(uint recordType = 0; recordType < BPStats::Size; ++recordType)
  {
    if(!mCheckBoxes[recordType]->GetChecked())
      continue;

    Widget* graph = CreateRecordGraphWidget(type, tracker, recordType);
    Window* window = mEditor->AddManagedWidget(graph, DockArea::Right, true);
    window->SetTranslationAndSize(Pixels(200, 200, 0), Pixels(280, 400));

    mActiveWindows.PushBack(window);
  }
}

Widget* BroadPhaseEditor::CreateRecordGraphWidget(BroadPhase::Type type, 
         BroadPhaseTracker* tracker, BPStats::Type recordType)
{
  // Create a graph
  GraphView* graph = new GraphView(mEditor);
  graph->SetName(BPStats::Names[recordType]);

  // Add a sampler for each broad phase
  for(uint i = 0; i < mActiveBroadPhases[type].Size(); ++i)
  {
    // Pull the statistics from the tracker
    Statistics* stats = tracker->GetStatistics(type, i);

    // Get the record
    Profile::Record* record = stats->GetRecord(recordType);

    // Add the record to the graph
    String name = mActiveBroadPhases[type][i];
    graph->AddSampler(new RecordSampler(name, record));
  }

  return graph;
}

Widget* BroadPhaseEditor::CreateStatsGraphWidget(BroadPhase::Type type, 
                                                 BroadPhaseTracker* tracker)
{
  // Create a graph
  GraphView* graph = new GraphView(mEditor);
  graph->SetName("False Positives");

  // Add a sampler for each broad phase
  for(uint i = 0; i < mActiveBroadPhases[type].Size(); ++i)
  {
    // Pull the statistics from the tracker
    Statistics* stats = tracker->GetStatistics(type, i);

    // Create the statistics sampler
    String name = mActiveBroadPhases[type][i];
    StatisticsSampler* statsGraph = new StatisticsSampler(name, stats);

    // Add the sampler to the graph
    graph->AddSampler(statsGraph);
  }

  return graph;
}

Composite* BroadPhaseEditor::BuildPropertyGrid(StringParam label, 
                                               BroadPhase::Type type, 
                                               Composite* parent)
{
  // Create the encasing composite
  Composite* composite = new Composite(parent);
  composite->SetSize(Pixels(150, 54));
  composite->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  composite->SetLayout(CreateStackLayout());
  {
    // Create a text label
    Label* labelComposite = new Label(composite);
    labelComposite->SetText(label);
    labelComposite->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);

    Composite* addComposite = new Composite(composite);
    addComposite->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
    addComposite->SetLayout(CreateRowLayout());
    {
      // Create the combo box
      ComboBox* comboBox = new ComboBox(addComposite);
      comboBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);

      // Enumerate the names
      Z::gBroadPhaseLibrary->EnumerateNamesOfType(type, mBroadPhaseNames[type].Strings);

      // Set the data source
      comboBox->SetListSource(&mBroadPhaseNames[type]);
      comboBox->SetSelectedItem(0, false);

      // Store the combo box
      mComboBoxes[type] = comboBox;

      Spacer* spacer = new Spacer(composite);
      spacer->SetSize(Pixels(3, 1));

      // Create the add button
      IconButton* button = new IconButton(addComposite);
      button->SetIcon("BigPlus");
      button->SetToolTip("Add Broad Phase");
      
      // Store the button
      mAddButtons[type] = button;
    }


    // Create a horizontal spacer
    Spacer* spacer = new Spacer(composite);
    spacer->SetSize(Pixels(1, 1));

    Composite* removeListComposite = new Composite(composite);
    removeListComposite->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
    removeListComposite->SetLayout(CreateRowLayout());
    {
      // Create the list box to display the active broad phases
      ListBox* listBox = new ListBox(removeListComposite);
      // Create the data source for the list box
      listBox->SetDataSource(&mActiveBroadPhases[type]);
      listBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
      listBox->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);

      // Store the list box
      mListBoxes[type] = listBox;

      // Create a remove button
      IconButton* button = new IconButton(removeListComposite);
      button->SetIcon("RedX");
      button->SetToolTip("Remove Broad Phase");

      // Store the button
      mRemoveButtons[type] = button;
    }

  }

  return composite;
}

void BroadPhaseEditor::AddButtonPressed(BroadPhase::Type type)
{
  // If there are no more items to add, do nothing
  if(mBroadPhaseNames[type].Empty())
    return;

  // Get the combo box
  ComboBox* comboBox = mComboBoxes[type];

  // Get the index of the selected broad phase
  int selectedItem = comboBox->GetSelectedItem();

  // Do nothing if there is nothing selected
  if(selectedItem == -1)
    return;

  // Get the name of the selected broad phase
  String name = mBroadPhaseNames[type][selectedItem];

  // Add the selected item to the active broad phases
  mActiveBroadPhases[type].Strings.PushBack(name);

  // Remove it from the names list
  mBroadPhaseNames[type].Strings.EraseAt(selectedItem);

  // Update the combo box
  comboBox->SetSelectedItem(0, false);
  MarkAsNeedsUpdate();
}

void BroadPhaseEditor::DynamicAddButtonPressed(ObjectEvent* event)
{
  AddButtonPressed(BroadPhase::Dynamic);
}

void BroadPhaseEditor::StaticAddButtonPressed(ObjectEvent* event)
{
  AddButtonPressed(BroadPhase::Static);
}

void BroadPhaseEditor::RemoveButtonPressed(BroadPhase::Type type)
{
  // If there are no more items to remove, do nothing
  if(mActiveBroadPhases[type].Empty())
    return;

  // Get the list box
  ListBox* listBox = mListBoxes[type];

  // Get the index of the selected broad phase
  int selectedItem = listBox->GetSelectedItem();

  // Do nothing if there is nothing selected
  if(selectedItem == -1)
    return;

  // Get the name of the selected broad phase
  String name = mActiveBroadPhases[type][selectedItem];

  // Add the selected item to the active broad phases
  mBroadPhaseNames[type].Strings.PushBack(name);

  // Remove it from the names list
  mActiveBroadPhases[type].Strings.EraseAt(selectedItem);

  // Update the combo box
  listBox->SetSelectedItem(0, false);
  listBox->MarkAsNeedsUpdate(true);

  MarkAsNeedsUpdate();
}

void BroadPhaseEditor::DynamicRemoveButtonPressed(ObjectEvent* event)
{
  RemoveButtonPressed(BroadPhase::Dynamic);
}

void BroadPhaseEditor::StaticRemoveButtonPressed(ObjectEvent* event)
{
  RemoveButtonPressed(BroadPhase::Static);
}

void BroadPhaseEditor::GameSpaceDestroyed(ObjectEvent* event)
{
  for(uint i = 0; i < mActiveWindows.Size(); ++i)
    mActiveWindows[i].SafeDestroy();
}

}//namespace Zero
