///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class WidgetChildId
{
public:
  WidgetChildId();

  String mName;
  BoundType* mType;
  size_t mIndex;
};

class WidgetPath
{
public:
  WidgetPath();
  WidgetPath(Widget* toWidget, RootWidget* fromRoot);

  Widget* Resolve(RootWidget* root);

  Array<WidgetChildId> mPath;
};

class UnitTestSystem;

class UnitTestEvent
{
public:
  virtual ~UnitTestEvent();
  virtual void Execute(UnitTestSystem* system) = 0;
};

class UnitTestBaseMouseEvent : public UnitTestEvent
{
public:
  WidgetPath mWidgetPath;
  Vec2 mNormalizedWidgetOffset;
};

class UnitTestMouseEvent : public UnitTestBaseMouseEvent
{
public:
  // UnitTestEvent interface
  void Execute(UnitTestSystem* system) override;

  OsMouseEvent mEvent;
};

class UnitTestMouseDropEvent : public UnitTestBaseMouseEvent
{
public:
  // UnitTestEvent interface
  void Execute(UnitTestSystem* system) override;

  OsMouseDropEvent mEvent;
};

class UnitTestKeyboardEvent : public UnitTestEvent
{
public:
  // UnitTestEvent interface
  void Execute(UnitTestSystem* system) override;

  KeyboardEvent mEvent;
};

class UnitTestKeyboardTextEvent : public UnitTestEvent
{
public:
  // UnitTestEvent interface
  void Execute(UnitTestSystem* system) override;

  KeyboardTextEvent mEvent;
};

class UnitTestWindowEvent : public UnitTestEvent
{
public:
  // UnitTestEvent interface
  void Execute(UnitTestSystem* system) override;

  OsWindowEvent mEvent;
};

class UnitTestFrame
{
public:
  Array<UnitTestEvent*> mEvents;
};

DeclareEnum3(UnitTestMode, Stopped, Recording, Playing);

class UnitTestSystem : public System
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  UnitTestSystem();

  // System interface
  cstr GetName();
  void Initialize(SystemInitializer& initializer) override;
  void Update() override;

  void StartUnitTestRecording(StringParam zeroTestFile);
  void PlayUnitTestRecording(StringParam zeroTestFile);

  // These should only be called while systems are NOT updating, especially not the OsShell systems
  // Calling these during the OsShell would result in loss of inputs / missed events
  void StartUnitTestRecordingSubProcess();
  void PlayUnitTestRecordingSubProcess();

  RootWidget* GetRootWidget();
  OsWindow* GetMainWindow();

  // Internals
  Widget* mEmulatedCursor;
  UnitTestMode::Enum mMode;
  size_t mFrameIndex;
  Array<UnitTestFrame*> mFrames;

  // Event callbacks
  void RecordMouseEvent(OsMouseEvent* event);
  void RecordMouseFileDropEvent(OsMouseDropEvent* event);
  void RecordKeyboardEvent(KeyboardEvent* event);
  void RecordKeyboardTextEvent(KeyboardTextEvent* event);
  void RecordWindowEvent(OsWindowEvent* event);

  void RecordBaseMouseEvent(UnitTestBaseMouseEvent* baseEvent, OsMouseEvent* event);
  void ExecuteBaseMouseEvent(UnitTestBaseMouseEvent* baseEvent, OsMouseEvent* event);
};

UnitTestSystem* CreateUnitTestSystem();

}// namespace Zero
