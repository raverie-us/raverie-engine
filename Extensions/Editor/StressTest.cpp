///////////////////////////////////////////////////////////////////////////////
///
/// \file StressTest.cpp
/// Implementation of the StressTest class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String ToString(Cog* sourceCog)
{
  StringBuilder output;

  // Get the cog-id string
  CogId id = sourceCog;
  String cogIdStr = ToString(id.ToUint64());

  // Print out the console
  output.Append("Cog \"");
  output.Append(sourceCog->GetName());
  output.Append("\" [");
  if(Archetype* archetype = sourceCog->GetArchetype())
    output.Append(archetype->Name);
  output.Append("] <");
  output.Append(cogIdStr);
  output.Append("> ");

  // Get the components
  Cog::ComponentRange range = sourceCog->GetComponents();

  // Loop through the components
  while (range.Empty() == false)
  {
    // Write the name of the component
    BoundType* componentType = ZilchVirtualTypeId(range.Front());
    output.Append(componentType->Name);

    // Iterate to the next component
    range.PopFront();

    // If we have components...
    if (range.Empty() == false)
    {
      // Add another comma to separate the names
      output.Append(", ");
    }
  }

  return output.ToString();
}

// HACK
StressTestDialog* gStressTestDialog = NULL;
bool StressTestErrorHandler(ErrorSignaler::ErrorData& errorData)
{
  uint len = strlen(errorData.Message);
  char* message = (char*)errorData.Message;

  StringBuilder error;
  error.Append("Error: ");
  error.Append("'");
  error.Append(message);
  error.Append("', '");
  error.Append(errorData.Expression);
  error.Append("', '");
  error.Append(errorData.File);
  error.Append("', line ");
  error.Append(ToString(errorData.Line));

  gStressTestDialog->Log("%s", error.ToString().c_str());
  return false;
}

// Occurs when the engine updates
void OnUpdate(ObjectEvent* event);

//------------------------------------------------------------------------ StressRandom
// Constructor
StressRandom::StressRandom(int seed)
  : mRandom(seed)
{
  //
}

// Generate a random float from 0 to 1
float StressRandom::RandomFloat()
{
  return mRandom.Float();
}

// Generate a random float within a range
float StressRandom::RandomFloatRange(float min, float max)
{
  return mRandom.FloatRange(min, max);
}

// Generate a random float with a variance
float StressRandom::RandomFloatVariance(float base, float variance)
{
  return mRandom.FloatVariance(base, variance);
}

// Generate a random integer
int StressRandom::RandomInt()
{
  return int(mRandom.Uint32());
}

// Generate a random integer between [0, max)
int StressRandom::RandomIntMax(int max)
{
  return RandomInt() % max;
}

// Generate a random integer between [min, max)
int StressRandom::RandomIntRange(int min, int max)
{
  ErrorIf(min > max, "Invalid range.");
  int range = max - min;
  return (RandomInt() % range) + min;
}

// Generate a random unsigned integer
uint StressRandom::RandomUint()
{
  return mRandom.Uint32();
}

// Generate a random unsigned integer
uint StressRandom::RandomUintMax(int max)
{
  return RandomUint() % max;
}

// Generate a random unsigned integer
uint StressRandom::RandomUintRange(int min, int max)
{
  ErrorIf(min > max, "Invalid range.");
  uint range = max - min;
  return (RandomUint() % range) + min;
}

// Generate a random boolean
bool StressRandom::RandomBool()
{
  return (RandomInt() % 2) != 0;
}

// Generate a random position
Vec2 StressRandom::RandomPosition2d()
{
  Vec2 position;
  position.x = (RandomFloat() - 0.5f) * 100.0f;
  position.y = (RandomFloat() - 0.5f) * 100.0f;
  return position;
}

// Generate a random position
Vec3 StressRandom::RandomPosition3d()
{
  Vec3 position;
  position.x = (RandomFloat() - 0.5f) * 100.0f;
  position.y = (RandomFloat() - 0.5f) * 100.0f;
  position.z = (RandomFloat() - 0.5f) * 100.0f;
  return position;
}

// Generate a random position
Vec4 StressRandom::RandomPosition4d()
{
  Vec4 position;
  position.x = (RandomFloat() - 0.5f) * 100.0f;
  position.y = (RandomFloat() - 0.5f) * 100.0f;
  position.z = (RandomFloat() - 0.5f) * 100.0f;
  position.w = (RandomFloat() - 0.5f) * 100.0f;
  return position;
}

// Generate a random direction
Vec3 StressRandom::RandomDirection()
{
  const float cTwoPi = float(Math::cTwoPi);
  float theta = cTwoPi * RandomFloat();
  float phi = Math::ArcCos(real(2.0f * RandomFloat() - 1.0f));

  real sinPhi = Math::Sin(phi);
  return Vec3(Math::Cos(real(theta)) * sinPhi,
              Math::Sin(real(theta)) * sinPhi,
              Math::Cos(real(phi)));
}

// Generate a random scale
Vec3 StressRandom::RandomScale()
{
  Vec3 scale;
  scale.x = RandomFloat() + 0.5f;
  scale.y = RandomFloat() + 0.5f;
  scale.z = RandomFloat() + 0.5f;
  return scale;
}

// Generate a random rotation
Quat StressRandom::RandomRotation()
{
  return mRandom.RotationQuaternion();
}

// StressRandom chance
bool StressRandom::Chance(float percentage)
{
  return RandomFloat() <= percentage;
}

// Generate a random string
String StressRandom::RandomString()
{
  StringBuilder builder;

  int length = RandomInt() % 20 + 10;

  for (int i = 0; i < length; ++i)
  {
    builder.Append((char)(RandomInt() % 255 + 1));
  }

  return builder.ToString();
}

//------------------------------------------------------------------------ StressTest
ZilchDefineType(StressTest, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindFieldProperty(Seed);
  ZilchBindFieldProperty(Frames);
  ZilchBindFieldProperty(LogFile);

  ZilchBindFieldProperty(CreateObjects);
  ZilchBindFieldProperty(DestroyObjects);
  ZilchBindFieldProperty(SetProperties);
  ZilchBindFieldProperty(AddComponents);
  ZilchBindFieldProperty(RemoveComponents);
  ZilchBindFieldProperty(ChangeSelection);
  ZilchBindFieldProperty(RayCasts);
  ZilchBindFieldProperty(ParentObjects);
  ZilchBindFieldProperty(UnparentObjects);
  ZilchBindFieldProperty(MouseEvents);
  ZilchBindFieldProperty(KeyEvents);
  ZilchBindFieldProperty(ChangeTools);
  ZilchBindFieldProperty(StartGameInstances);
  ZilchBindFieldProperty(StopGameInstances);
  ZilchBindFieldProperty(SwitchViewports);
  ZilchBindFieldProperty(Undo);
  ZilchBindFieldProperty(Redo);
  //ZilchBindFieldProperty(CreateObjectLinks)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(ModifyArchetypes)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(CreateResources)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(LoadResources)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(ResourceProperties)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(ConfigProperties)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(ProjectProperties)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(RunCommands)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(ShapeCasts)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(CollisionPairs)->SetFlag(PropertyFlags::Serialized);
  //ZilchBindFieldProperty(LightingModes)->SetFlag(PropertyFlags::Serialized);
}

// Constructor
StressTest::StressTest()
{
  Seed = 1234;
  Frames = 1000;
  LogFile = FilePath::Combine(GetUserDocumentsDirectory(), "Log.txt");
  CreateObjects = true;
  DestroyObjects = true;
  SetProperties = true;
  AddComponents = true;
  RemoveComponents = true;
  ChangeSelection = false;
  RayCasts = true;
  ParentObjects = false;
  UnparentObjects = false;
  MouseEvents = false;
  KeyEvents = false;
  ChangeTools = false;
  StartGameInstances = false;
  StopGameInstances = false;
  SwitchViewports = false;
  Undo = false;
  Redo = false;
  //CreateObjectLinks         = false;
  //ModifyArchetypes          = false;
  //CreateResources           = false;
  //LoadResources             = false;
  //ResourceProperties        = false;
  //ConfigProperties          = false;
  //ProjectProperties         = false;
  //RunCommands               = false;
  //ShapeCasts                = false;
  //CollisionPairs            = false;
  //LightingModes            = false;
}

void StressTest::Serialize(Serializer& stream)
{
  MetaSerializeProperties(this, stream);

  if(LogFile.Empty())
    LogFile = FilePath::Combine(GetUserDocumentsDirectory(), "Log.txt");
}

//------------------------------------------------------------------------ StressTestDialog
ZilchDefineType(StressTestDialog, builder, type)
{
}

// Constructor
StressTestDialog::StressTestDialog(Composite* parent) :
  Composite(parent),
  mRandom(0),
  mTestIterationCount(0),
  mTestFrame(0)
{
  SetLayout(CreateStackLayout());
  SetMinSize(Vec2(300, 300));

  mSettingsLocation = FilePath::Combine(GetUserDocumentsDirectory(), "StressTester.txt");
  if(FileExists(mSettingsLocation))
    LoadFromDataFile(mTestInfo, mSettingsLocation);

  // Start the test
  mStartTest = new TextButton(this);
  mStartTest->SetText("Start Test");
  ConnectThisTo(mStartTest, Events::ButtonPressed, OnStartTest);

  mPropertyEditor = new PropertyView(this);
  mPropertyEditor->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mPropertyEditor->SetObject(&mTestInfo);

  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);

  mStarted = false;

  mExtensionFunctions[ZilchTypeId(Resource)] = &StressTestDialog::GetVariantForResourceExtension;
  mExtensionFunctions[ZilchTypeId(Enum)] = &StressTestDialog::GetVariantForEnumExtension;
  mExtensionFunctions[ZilchTypeId(EditorRange)] = &StressTestDialog::GetVariantForRangeExtension;
  mExtensionFunctions[ZilchTypeId(EditorIndexedStringArray)] = &StressTestDialog::GetVariantForStringArray;

  mIgnoreObjects.PushBack(SpecialCogNames::WorldAnchor);
  mIgnoreObjects.PushBack("Renderer");
  mIgnoreObjects.PushBack("Camera");
  mIgnoreObjects.PushBack(SpecialCogNames::EditorCamera);

  mIgnoreComponents.PushBack("CSGTester");
  mIgnoreComponents.PushBack("SubSpaceMount");

  mIgnoreComponents.PushBack("Hierarchy");
  mIgnoreComponents.PushBack("NavGrid");
}

void StressTestDialog::Refresh()
{
  mPropertyEditor->SetObject(&mTestInfo);
}

// Occurs when we click the start test button
void StressTestDialog::OnStartTest(ObjectEvent* event)
{
  mTestIterationCount = 0;
  mTestFrame = 0;

  Cog* projectCog = Z::gEditor->mProject;
  if (projectCog == NULL)
    return;

  Z::gEditor->SaveAll(true);
  ProjectSettings* project = projectCog->has(ProjectSettings);
  String projectDirectory = project->ProjectFolder;
  String projName = project->ProjectName;
  FilterFileRegex emptyFilter = FilterFileRegex(String(), String());

  String dest = FilePath::Combine(GetTemporaryDirectory(), projName);

  if (dest != projectDirectory)
  {
    MoveFolderContents(GetEditorTrash(), dest, &emptyFilter);
    CopyFolderContents(dest, projectDirectory, &emptyFilter);
  }

  String newProjLocation = FilePath::Combine(dest, BuildString(projName, ".zeroproj"));

  Z::gEditor->mConfig->has(MainConfig)->mSave = false;

  //OpenProjectFile(newProjLocation);

  SaveToDataFile(mTestInfo, mSettingsLocation);

  //hide this window so we can't modify while running
  this->SetActive(false);

  mStarted = true;
  mRandom = StressRandom(mTestInfo.Seed);

  if (mLogFile.IsOpen())
  {
    mLogFile.Close();
  }

  //HACK
  gStressTestDialog = this;
  ErrorSignaler::SetErrorHandler(StressTestErrorHandler);

  bool opened = mLogFile.Open(mTestInfo.LogFile.c_str(), FileMode::Write, FileAccessPattern::Sequential);
  ErrorIf(!opened, "Failed to open file for text output");

  Log("Seed: %d", mTestInfo.Seed);
}

// Occurs when the engine updates
void StressTestDialog::OnUpdate(UpdateEvent* event)
{
  if (mStarted == false)
    return;

  int iterationsPerFrame = mRandom.RandomIntMax(3);

  for (int frame = 0; frame < iterationsPerFrame; ++frame)
  {
    TestOnce();
  }

  ++mTestFrame;
  --mTestInfo.Frames;

  if (mTestInfo.Frames == 0)
    mStarted = false;
}

// Log something to the log file
void StressTestDialog::Log(cstr format, ...)
{
  if (!mLogFile.IsOpen())
    mLogFile.Open(mTestInfo.LogFile.c_str(), FileMode::Append, FileAccessPattern::Sequential);

  if (mLogFile.IsOpen())
  {
    va_list args;
    va_start(args, format);
    String logLine = String::FormatArgs(format, args);
    cstr newLine = "\r\n";
    mLogFile.Write((byte*)logLine.Data(), logLine.SizeInBytes());
    mLogFile.Write((byte*)newLine, strlen(newLine));
    mLogFile.Flush();
    va_end(args);
    mLogFile.Close();
  }
}

// Generate a random variant
Any StressTestDialog::RandomVariantOfType(BoundType* boundType, Space* targetSpace)
{
  if (boundType == ZilchTypeId(String))
    return mRandom.RandomString();
  else if (boundType == ZilchTypeId(int))
    return mRandom.RandomInt();
  // uint has no type. Can't support for now
  //else if(boundType == ZilchTypeId(uint))
  //  return mRandom.RandomUint();
  else if (boundType == ZilchTypeId(bool))
    return mRandom.RandomBool();
  else if (boundType == ZilchTypeId(float))
    return mRandom.RandomFloatRange(-50.0f, 50.0f);
  else if (boundType == ZilchTypeId(Vec2))
    return mRandom.RandomPosition2d();
  else if (boundType == ZilchTypeId(Vec3))
    return mRandom.RandomPosition3d();
  else if (boundType == ZilchTypeId(Vec4))
    return mRandom.RandomPosition4d();
  else if (boundType == ZilchTypeId(Quat))
    return mRandom.RandomRotation();

  if (boundType == ZilchTypeId(CogPath))
    return CogPath(mRandom.RandomString());

  if (boundType == ZilchTypeId(Cog))
  {
    if (mRandom.mRandom.DieRoll(20))
      return (Cog*)nullptr;
    else
      return GetRandomObject(targetSpace);
  }

  Any defaultValue;
  defaultValue.DefaultConstruct(boundType);
  return defaultValue;
}

// Run a single test
void StressTestDialog::TestOnce()
{
  Space* targetSpace = Z::gEditor->GetEditSpace();

  if (targetSpace == NULL)
    return;

  Log("******************************************** Test(%d) on Frame(%d)", mTestIterationCount, mTestFrame);
  ++mTestIterationCount;

  OperationQueue* queue = Z::gEditor->GetOperationQueue();

  if (mTestInfo.CreateObjects)
  {
    Vec3 position = mRandom.RandomPosition3d();
    Quat rotation = mRandom.RandomRotation();
    Vec3 scale = mRandom.RandomScale();

    String archetype = mRandom.RandomString();

    Array<String> archetypes;
    ArchetypeManager::GetInstance()->EnumerateResources(archetypes);

    if (archetypes.Empty() == false && mRandom.Chance(0.9f))
    {
      archetype = archetypes[mRandom.RandomIntMax(archetypes.Size())];
    }

    String name = mRandom.RandomString();

    Log("Created '%s' at %s with rotation %s and scale %s named %s",
        archetype.c_str(),
        ToString(position).c_str(),
        ToString(rotation).c_str(),
        ToString(scale).c_str(),
        name.c_str());

    //TODO
    // Go through random archetypes (not just cube)
    // Occasionally try to create bad ones
    Cog* object = targetSpace->CreateAt(archetype, position, rotation, scale);

    if (object != NULL)
    {
      object->SetName(name);

      Log(" - Resulting Cog: %s", ToString(object).c_str());
    }
    else
    {
      Log(" - Failed to create object");
    }
  }

  if (mTestInfo.DestroyObjects)
  {
    if (Cog* object = GetRandomObject(targetSpace))
    {
      Log("Destroying %s", ToString(object).c_str());
      object->Destroy();

      if (object->GetMarkedForDestruction())
      {
        Log(" - Deleted");
      }
      else
      {
        Log(" - Not Deleted");
      }
    }
  }

  if (mTestInfo.SetProperties)
  {
    if (Component* component = GetRandomComponent(targetSpace))
    {
      BoundType* boundType = ZilchVirtualTypeId(component);

      Property* property = GetRandomSetProperty(boundType);
      if(property != nullptr)
      {
        Any oldVariant = property->GetValue(component);

        if (!property->IsReadOnly())
        {
          BoundType* propBoundType = Type::GetBoundType(property->PropertyType);
          ExtensionFunction function = nullptr;

          Any newVariant;

          if (EditorPropertyExtension* extension = property->HasInherited<EditorPropertyExtension>())
            function = mExtensionFunctions.FindValue(ZilchVirtualTypeId(extension), nullptr);

          // Check if this is an enum
          if(function == nullptr && property->PropertyType->IsA(ZilchTypeId(Enum)))
            function = &GetVariantForEnumExtension;
          // Check if this is a resource
          if(function == nullptr && property->PropertyType->IsA(ZilchTypeId(Resource)))
            function = &GetVariantForResourceExtension;

          if(function == nullptr)
          {
            MetaPropertyEditor* propertyEditor = property->PropertyType->HasInherited<MetaPropertyEditor>();
            BoundType* propertyEditorType = ZilchVirtualTypeId(propertyEditor);
            if(propertyEditorType)
              function = mExtensionFunctions.FindValue(propertyEditorType, nullptr);
          }

          if (function != nullptr)
            newVariant = function(mRandom, property, component);
          else
            newVariant = RandomVariantOfType(Type::GetBoundType(property->PropertyType), targetSpace);

          Log("Set Property '%s' on '%s' (old value %s, new value %s) on Cog %s",
              property->Name.c_str(),
              ZilchVirtualTypeId(component)->Name.c_str(),
              oldVariant.ToString().c_str(),
              newVariant.ToString().c_str(),
              ToString(component->GetOwner()).c_str());

          Any result = property->GetValue(component);

          Log(" - Resulting property value was %s", result.ToString().c_str());

          ChangeAndQueueProperty(queue, component, property, newVariant);
        }
      }
    }
  }

  if (mTestInfo.AddComponents)
  {
    if (Cog* object = GetRandomObject(targetSpace))
    {
      auto meta = ZilchVirtualTypeId(object);
      auto composition = meta->HasInherited<MetaComposition>();

      Array<String> availableComponents;
      composition->Enumerate(availableComponents, EnumerateAction::AllAddableToObject, object);

      if (!availableComponents.Empty())
      {
        String component = availableComponents[mRandom.RandomIntMax(availableComponents.Size())];

        bool validComponent = true;

        for (uint j = 0; j < mIgnoreComponents.Size(); ++j)
        {
          if (component == mIgnoreComponents[j])
          {
            validComponent = false;
            break;
          }
        }

        if (validComponent)
        {
          Log("Adding component %s on %s", component.c_str(), ToString(object).c_str());

          QueueAddComponent(queue, object, component);

          Log(" - Component %s added", component.c_str());
        }
      }
    }
  }

  if (mTestInfo.RemoveComponents)
  {
    if (Component* component = GetRandomComponent(targetSpace))
    {
      auto componentMeta = ZilchVirtualTypeId(component);
      auto owner = component->GetOwner();

      Log("Removing component %s from %s", componentMeta->Name.c_str(), ToString(owner).c_str());

      auto meta = ZilchVirtualTypeId(owner);

      bool wasRemoved = QueueRemoveComponent(queue, owner, componentMeta);

      if (wasRemoved)
      {
        Log(" - Component was removed");
      }
      else
      {
        Log(" - Component was not removed");
      }
    }
  }

  if (mTestInfo.ChangeSelection)
  {
    MetaSelection* selection = Z::gEditor->GetSelection();

    if (selection != NULL)
    {
      if (mRandom.Chance(0.1f))
      {
        selection->Clear();
      }
      else
      {
        if (mRandom.Chance(0.2f))
        {
          selection->SetPrimary(GetRandomObject(targetSpace));
        }
        else
        {
          selection->Add(GetRandomObject(targetSpace));
        }
      }
    }
  }

  if (mTestInfo.RayCasts)
  {
    PhysicsSpace* physics = targetSpace->has(PhysicsSpace);

    if (physics != NULL)
    {
      CastResults results(1 + mRandom.RandomIntMax(6));
      Ray worldRay;
      worldRay.Start = mRandom.RandomPosition3d();
      worldRay.Direction = mRandom.RandomDirection();
      physics->CastRay(worldRay, results);

      Log("Casting ray from %s in direction %s", ToString(worldRay.Start).c_str(), ToString(worldRay.Direction).c_str());

      StringBuilder castStr;

      for (uint i = 0; i < results.Size(); ++i)
      {
        auto hit = results[i];
        Log(" - Hit at %f, object %s", hit.GetDistance(), ToString(hit.GetObjectHit()).c_str());
      }
    }
  }

  if (mTestInfo.ParentObjects)
  {

    Cog* parent = GetRandomObject(targetSpace);
    Cog* child = GetRandomObject(targetSpace);

    if (parent && child && !child->mFlags.IsSet(CogFlags::Protected))
    {
      Log("Attaching %s to %s", ToString(child).c_str(), ToString(parent).c_str());

      bool relative = mRandom.RandomBool();
      AttachObject(queue, child, parent, relative);

      if (child->mHierarchyParent == parent)
        Log("Result - Attached");
      else
        Log("Result - Not Attached");
    }
  }

  if (mTestInfo.UnparentObjects)
  {
    Cog* child = GetRandomObject(targetSpace);

    if (child)
    {
      if (child->mHierarchyParent)
        Log("Detaching %s from parent %s", ToString(child).c_str(), ToString(child->mHierarchyParent).c_str());
      else
        Log("Detaching %s from parent", ToString(child).c_str());

      bool relative = mRandom.RandomBool();
      DetachObject(queue, child, relative);

      if (child->mHierarchyParent == NULL)
        Log("Result - Detached");
      else
        Log("Result - Not Detached");
    }
  }

  if (mTestInfo.MouseEvents)
  {
    //Needs updating
  }

  if (mTestInfo.KeyEvents)
  {
    KeyboardEvent event;

    event.AltPressed = mRandom.RandomBool();
    event.Key = (Keys::Enum)mRandom.RandomIntRange(0, Keys::KeyMax);
    event.State = (KeyState::Enum)mRandom.RandomIntRange(0, KeyState::Repeated);
    event.ShiftPressed = mRandom.RandomBool();
    event.AltPressed = mRandom.RandomBool();
    event.CtrlPressed = mRandom.RandomBool();
    event.SpacePressed = mRandom.RandomBool();
    event.Handled = mRandom.RandomBool();
    event.mKeyboard = Keyboard::GetInstance();

    OsShell* shell = Z::gEngine->has(OsShell);

    String eventId;
    if (mRandom.RandomBool())
      eventId = Events::OsKeyDown;
    else
      eventId = Events::OsKeyUp;

    shell->DispatchEvent(eventId, &event);
  }

  if (mTestInfo.ChangeTools)
  {
    Z::gEditor->Tools->SelectToolIndex(mRandom.RandomIntMax(Z::gEditor->Tools->mTools.mToolArray.Size() + 5));
  }

  if (mTestInfo.StartGameInstances)
  {
    if (mRandom.Chance(0.01f))
    {
      if (mRandom.RandomBool())
        Z::gEditor->PlayGame(PlayGameOptions::SingleInstance);
      else
        Z::gEditor->PlayGame(PlayGameOptions::MultipleInstances);
    }
  }

  if (mTestInfo.StopGameInstances)
  {
    if (mRandom.Chance(0.01f))
    {
      Z::gEditor->StopGame();
    }
  }

  if (mTestInfo.SwitchViewports)
  {
    if (mRandom.Chance(0.01f))
    {
      Composite* c = Z::gEditor;
      auto children = c->GetChildren();
      //grab all tab areas under the main window
      Array<TabArea*> tabAreas;
      for (; !children.Empty(); children.PopFront())
      {
        Widget& child = children.Front();
        //unfortunately the meta of the window is null now, I should fix this but
        //I'm lazy and I'll just check for the name window right now...
        if (ZilchVirtualTypeId(&child)->IsA(ZilchTypeId(Window)) || child.GetName() == "Window")
        {
          Window* window = static_cast<Window*>(&child);
          tabAreas.PushBack(window->mTabArea);
        }
      }

      if (tabAreas.Size() != 0)
      {
        uint sourceTabAreaIndex = mRandom.RandomIntMax(tabAreas.Size());
        uint destTabAreaIndex = mRandom.RandomIntMax(tabAreas.Size());

        TabArea* sourceTabArea = tabAreas[sourceTabAreaIndex];
        TabArea* destTabArea = tabAreas[destTabAreaIndex];

        if (sourceTabArea->mTabs.Size() != 0)
        {
          uint sourceTabIndex = mRandom.RandomIntMax(sourceTabArea->mTabs.Size());

          TabWidget* sourceTab = sourceTabArea->mTabs[sourceTabIndex];
          Vec2 pos = Math::ToVector2(sourceTab->mTitle->GetScreenPosition());

          OsMouseEvent event;
          event.ShiftPressed = false;
          event.AltPressed = false;
          event.CtrlPressed = false;
          //          event.DoubleClicked = false;

          //          event.Movement = Vec2::cZero;
          //          event.Position = pos;
          //         event.ScrollMovement = Vec2::cZero;
          //          event.State[MouseButtons::Left] = true;//OsMouseButtonState::Pressed;

          //send a down event
          OsShell* shell = Z::gEngine->has(OsShell);
          shell->DispatchEvent(Events::OsMouseDown, &event);

          //occasionally try to drag to another tab
          if (mRandom.Chance(0.3f) && destTabArea->mTabs.Size() != 0)
          {
            TabWidget* destTab = destTabArea->mTabs[0];
            Vec2 newPos;

            //either drag to another tab
            float chance = mRandom.RandomFloat();
            if (chance <= 0.6f)
              newPos = Math::ToVector2(destTab->mTitle->GetScreenPosition()) + destTab->mTitle->GetSize() * 0.5f;
            //or drag to one of the four edges of the window
            else
            {
              Vec2 size = c->GetSize();
              if (chance <= 0.7f)
                newPos = Vec2(size.x / 2, 0.0f);
              else if (chance <= 0.8f)
                newPos = Vec2(size.x / 2, size.y);
              else if (chance <= 0.9f)
                newPos = Vec2(0, size.y / 2);
              else
                newPos = Vec2(size.x, size.y / 2);
            }

            //now send a move to the new spot
            //            event.Movement = newPos - pos;
            //            event.Position = newPos;
            //            event.State[MouseButtons::Left] = true;//OsMouseButtonState::Held;
            //            shell->DispatchEvent(Events::OsMouseMove, &event);
            //a move has to be sent twice (at least on the same frame) otherwise it
            //doesn't work right now, I guess it's a bug in ui that needs to be fixed
            //            event.Movement = Vec2::cZero;
            //            shell->DispatchEvent(Events::OsMouseMove, &event);
          }

          //now send the corresponding up wherever we left off at
          //          event.Movement = Vec2::cZero;
          //          event.State[MouseButtons::Left] = OsMouseButtonState::Released;
          shell->DispatchEvent(Events::OsMouseUp, &event);
        }
      }
    }
  }

  if (mTestInfo.Undo)
  {
    if (mRandom.Chance(0.1f))
    {
      queue->Undo();
      Log("Undo");
    }
  }

  if (mTestInfo.Redo)
  {
    if (mRandom.Chance(0.1f))
    {
      queue->Redo();
      Log("Redo");
    }
  }
}

Cog* StressTestDialog::GetRandomObject(Space* targetSpace)
{
  if (targetSpace->GetObjectCount() > 0)
  {
    int index = mRandom.RandomIntMax(targetSpace->GetObjectCount());
    auto objects = targetSpace->AllObjects();

    for (int i = 0; i < index; ++i)
    {
      objects.PopFront();
    }

    Cog* cog = &objects.Front();

    for (uint j = 0; j < mIgnoreObjects.Size(); ++j)
    {
      if (cog->GetName() == mIgnoreObjects[j] || cog->mFlags.IsSet(CogFlags::ObjectViewHidden))
      {
        return NULL;
      }
    }

    return cog;
  }

  return NULL;
}

// Get a random component
Component* StressTestDialog::GetRandomComponent(Space* targetSpace)
{
  Cog* object = nullptr;
  // Attempt a max number of times to get a valid object
  for(size_t i = 0; i < 1000; ++i)
  {
    object = GetRandomObject(targetSpace);
    if(object != nullptr)
      break;
  }

  if (object != nullptr)
  {
    auto components = object->GetComponents();

    if (components.Empty() == false)
    {
      Component* component = components[mRandom.RandomIntMax(components.Size())];

      for (uint j = 0; j < mIgnoreComponents.Size(); ++j)
      {
        BoundType* componentType = ZilchVirtualTypeId(component);
        if (componentType->Name == mIgnoreComponents[j])
        {
          return NULL;
        }
      }

      return component;
    }
  }

  return NULL;
}

Property* StressTestDialog::GetRandomSetProperty(BoundType* boundType)
{
  PropertyArray properties;
  // In order to get base type properties in an array (so we can randomly access)
  // we have to create a new array and copy all properties over.
  auto range = boundType->GetProperties(Zilch::Members::All);
  for(; !range.Empty(); range.PopFront())
    properties.PushBack(range.Front());

  if(properties.Empty())
    return nullptr;

  // Find a non-readonly property (give up after so many attempts)
  Property* property = nullptr;
  for(size_t i = 0; i < 1000; ++i)
  {
    property = properties[mRandom.RandomIntMax(properties.Size())];
    if(!property->IsReadOnly())
      break;
  }
  return property;
}

Any StressTestDialog::GetVariantForResourceExtension(StressRandom& random, Property* prop, Component* component)
{
  EditorResource* editorResource = prop->HasInherited<EditorResource>();
  // Get the resource manager for this editor resource type
  BoundType* resourceType = Type::GetBoundType(prop->PropertyType);
  ResourceManager* manager = Z::gResources->Managers.FindValue(resourceType->Name, NULL);
  // Get all of the resources for this resource type
  Array<String> resources;
  manager->EnumerateResources(resources);
  // There is a 90% chance that we will pick a pre-existing resource,
  // otherwise generate a random name
  String resourceName = random.RandomString();
  if(resources.Size() != 0)
    resourceName = resources[random.RandomIntMax(resources.Size())];
  
  Resource* resource = manager->GetResource(resourceName, ResourceNotFound::ErrorFallback);
  return resource;
}

Any StressTestDialog::GetVariantForEnumExtension(StressRandom& random, Property* prop, Component* component)
{
  BoundType* propertyType = Type::GetBoundType(prop->PropertyType);
  Zilch::PropertyArray& properties = propertyType->AllProperties;
  
  if(random.Chance(0.9f))
  {
    uint index = random.RandomIntMax(properties.Size());
    return Any(properties[index]);
  }
  uint index = random.RandomInt();
  return Any(index);
}

Any StressTestDialog::GetVariantForRangeExtension(StressRandom& random, Property* prop, Component* component)
{
  EditorRange* editorRange = prop->HasInherited<EditorRange>();
  BoundType* propertyType = Type::GetBoundType(prop->PropertyType);
  ErrorIf(propertyType->Name == "uint", "");
  // We want to generate a value within a range, but we have to determine the
  // value type of the range, there is also a 90% chance we will pick a
  // value within the valid range, a 10% chance we will pick a random value
  if(propertyType == ZilchTypeId(float))
  {
    float value = 0;
    if(random.Chance(0.9f))
      value = random.RandomFloat();
    else
      value = random.RandomFloatRange(editorRange->MinValue,editorRange->MaxValue);
    return Any(value);
  }
  // Uint has not type, can't support for now
  //else if(propertyType == ZilchTypeId(uint))
  //{
  //  if(random.Chance(0.9f))
  //    return Variant((uint)random.RandomInt());
  //  return Variant((uint)random.RandomIntRange((uint)editorRange->MinValue,(uint)editorRange->MaxValue));
  //}
  else if(propertyType == ZilchTypeId(int))
  {
    int value = 0;
    if(random.Chance(0.9f))
      value = random.RandomInt();
    else
      value = random.RandomIntRange((int)editorRange->MinValue, (int)editorRange->MaxValue);
    return Any(value);
  }
  else
    //there is a range type we don't handle
    Os::DebugBreak();
    
  return Any(0);
}

Any StressTestDialog::GetVariantForStringArray(StressRandom& random, Property* prop, Component* component)
{
  EditorIndexedStringArray* editorStrArray = prop->HasInherited<EditorIndexedStringArray>();
  
  if(random.Chance(0.7f))
  {
    Array<String> strs;
    editorStrArray->Enumerate(component, prop, strs);
    if(strs.Size() != 0)
      return Any(strs[random.RandomIntRange(0, strs.Size())]);
  }
  
  return Any(random.RandomString());
}

}//namespace Zero
