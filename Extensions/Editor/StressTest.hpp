///////////////////////////////////////////////////////////////////////////////
///
/// \file StressTest.hpp
/// Declaration of the StressTest class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
// Forward declarations
class PropertyView;
class TextButton;
class UpdateEvent;
class AddRemoveListBox;
struct StressTest;

// A class we use to randomly generate content
class StressRandom
{
public:
  // Constructor
  StressRandom(int seed);

  // Generate a random float from 0 to 1
  float RandomFloat();

  // Generate a random float within a range
  float RandomFloatRange(float min, float max);

  // Generate a random float with a variance
  float RandomFloatVariance(float base, float variance);

  // Generate a random integer
  int RandomInt();

  // Generate a random integer between [0, max)
  int RandomIntMax(int max);

  // Generate a random integer between [min, max)
  int RandomIntRange(int min, int max);

  // Generate a random unsigned integer
  uint RandomUint();

  // Generate a random unsigned integer
  uint RandomUintMax(int max);

  // Generate a random unsigned integer
  uint RandomUintRange(int min, int max);

  // Generate a random boolean
  bool RandomBool();

  // Generate a random position
  Vec2 RandomPosition2d();

  // Generate a random position
  Vec3 RandomPosition3d();

  // Generate a random position
  Vec4 RandomPosition4d();

  // Generate a random direction
  Vec3 RandomDirection();

  // Generate a random scale
  Vec3 RandomScale();

  // Generate a random rotation
  Quat RandomRotation();

  // Generate a random string
  String RandomString();

  // Random chance
  bool Chance(float percentage);

  // The internal random number generator
  Math::Random mRandom;
};

struct StressTest : public EventObject
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Constructor
  StressTest();

  void Serialize(Serializer& stream);

  int Seed;
  int Frames;
  String LogFile;
  bool CreateObjects;
  bool DestroyObjects;
  bool SetProperties;
  bool AddComponents;
  bool RemoveComponents;
  bool ChangeSelection;
  bool RayCasts;
  bool ParentObjects;
  bool UnparentObjects;
  bool MouseEvents;
  bool KeyEvents;
  bool ChangeTools;
  bool StartGameInstances;
  bool StopGameInstances;
  bool SwitchViewports;
  bool Undo;
  bool Redo;
  //bool CreateObjectLinks;
  //bool ModifyArchetypes;
  //bool CreateResources;
  //bool LoadResources;
  //bool ResourceProperties;
  //bool ConfigProperties;
  //bool ProjectProperties;
  //bool RunCommands;
  //bool ShapeCasts;
  //bool CollisionPairs;
  //bool LightingModes;
};

/// A dialog that we use to run stress tests
class StressTestDialog : public Composite
{
public:
  // Type-defines
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef StressTestDialog ZilchSelf;

  // Constructor
  StressTestDialog(Composite* parent);

  void Refresh();

  // Occurs when we click the start test button
  void OnStartTest(ObjectEvent* event);

  // Occurs when the engine updates
  void OnUpdate(UpdateEvent* event);

  // Log something to the log file
  void Log(cstr format, ...);

  // Generate a random variant
  Any RandomVariantOfType(BoundType* type, Space* targetSpace);

private:

  // Run a single test
  void TestOnce();

  // Get a random cog object
  Cog* GetRandomObject(Space* targetSpace);

  // Get a random component
  Component* GetRandomComponent(Space* targetSpace);
  Property* GetRandomSetProperty(BoundType* boundType);

  typedef Any (*ExtensionFunction)(StressRandom& random, Property* prop, Component* component);
  static Any GetVariantForResourceExtension(StressRandom& random, Property* prop, Component* component);
  static Any GetVariantForEnumExtension(StressRandom& random, Property* prop, Component* component);
  static Any GetVariantForRangeExtension(StressRandom& random, Property* prop, Component* component);
  static Any GetVariantForStringArray(StressRandom& random, Property* prop, Component* component);

private:

  // The editor for all the options
  PropertyView* mPropertyEditor;

  // Start the test
  TextButton* mStartTest;

  // All the test information
  StressTest mTestInfo;

  // Was the test started?
  bool mStarted;

  // What iteration are we on (how many tests have we ran)
  int mTestIterationCount;
  int mTestFrame;

  // A log file that we write events out to
  File mLogFile;

  // The random generator
  StressRandom mRandom;

  // A map to handle different extension types (resources, enums, etc)
  HashMap<BoundType*, ExtensionFunction> mExtensionFunctions;

  // A list of components to be ignored (by type name)
  Array<String> mIgnoreComponents;

  // A list of objects to be ignored (by name)
  Array<String> mIgnoreObjects;

  String mSettingsLocation;
};

}//namespace Zero
