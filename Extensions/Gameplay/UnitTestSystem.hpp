///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------------------------------ Widgets
class WidgetChildId
{
public:
  WidgetChildId();
  void Serialize(Serializer& stream);

  String mName;
  BoundType* mType;
  uint mIndex;
};

class WidgetPath
{
public:
  WidgetPath();
  WidgetPath(Widget* toWidget, RootWidget* fromRoot);
  
  void Serialize(Serializer& stream);
  Widget* Resolve(RootWidget* root);

  Array<WidgetChildId> mPath;
};

class UnitTestSystem;

//--------------------------------------------------------------------------------- Unit Test Events
class UnitTestEvent : public IZilchObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  virtual void Serialize(Serializer& stream) = 0;
  virtual ~UnitTestEvent();
  virtual void Execute(UnitTestSystem* system) = 0;
};

class UnitTestEndFrameEvent : public UnitTestEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;
};

class UnitTestBaseMouseEvent : public UnitTestEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;

  WidgetPath mWidgetPath;
  Vec2 mNormalizedWidgetOffset;
};

class UnitTestMouseEvent : public UnitTestBaseMouseEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  OsMouseEvent mEvent;
};

class UnitTestMouseDropEvent : public UnitTestBaseMouseEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  OsMouseDropEvent mEvent;
};

class UnitTestKeyboardEvent : public UnitTestEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  KeyboardEvent mEvent;
};

class UnitTestKeyboardTextEvent : public UnitTestEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  KeyboardTextEvent mEvent;
};

class UnitTestWindowEvent : public UnitTestEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // UnitTestEvent interface
  void Serialize(Serializer& stream) override;
  void Execute(UnitTestSystem* system) override;

  OsWindowEvent mEvent;
};

DeclareEnum5(UnitTestMode, Stopped, StartRecording, Recording, StartPlaying, Playing);

class UnitTestSystem : public System, public OsShellHook, public OsInputHook
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  UnitTestSystem();

  // System interface
  cstr GetName();
  void Initialize(SystemInitializer& initializer) override;

  // OsShellHook interface
  void HookUpdate();

  void RecordToZeroTestFile();
  void RecordToZeroTestFile(StringParam zeroTestFile);

  void PlayFromZeroTestFile();
  void PlayFromZeroTestFile(StringParam zeroTestFile);

  // These should only be called while systems are NOT updating, especially not the OsShell systems
  // Calling these during the OsShell would result in loss of inputs / missed events
  void SubProcessRecord();
  void SubProcessPlay();
  OsWindow* SubProcessSetupWindow();

  RootWidget* GetRootWidget();
  OsWindow* GetMainWindow();

  void OnUnitTestRecordFileSelected(OsFileSelection* event);
  void OnUnitTestPlayFileSelected(OsFileSelection* event);

  // Internals
  Widget* mEmulatedCursor;
  UnitTestMode::Enum mMode;
  Array<UnitTestEvent*> mEvents;

  void RecordBaseMouseEvent(UnitTestBaseMouseEvent* baseEvent, OsMouseEvent* event);
  void ExecuteBaseMouseEvent(UnitTestBaseMouseEvent* baseEvent, OsMouseEvent* event);

  void RecordEvent(UnitTestEvent* e);
  void LoadRecordedEvents();

  // InputHook interface
  void HookKeyboardEvent(KeyboardEvent& event) override;
  void HookKeyboardTextEvent(KeyboardTextEvent& event) override;
  void HookMouseEvent(OsMouseEvent& event) override;
  void HookMouseDropEvent(OsMouseDropEvent& event) override;
  void HookWindowEvent(OsWindowEvent& event) override;

  // The file we store our recorded data
  String mRecordedEventsDirectory;
  uint mEventIndex;
};

UnitTestSystem* CreateUnitTestSystem();

}// namespace Zero
