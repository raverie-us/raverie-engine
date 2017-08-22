///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------------------------ Widgets
//**************************************************************************************************
WidgetChildId::WidgetChildId() :
  mType(nullptr),
  mIndex(0)
{
}

//**************************************************************************************************
void WidgetChildId::Serialize(Serializer& stream)
{
  SerializeName(mName);
  
  if(stream.GetMode() == SerializerMode::Loading)
  {
    String typeName;
    stream.SerializeField("TypeName", typeName);
    mType = MetaDatabase::FindType(typeName);
  }
  else
  {
    if(mType)
      stream.SerializeField("TypeName", mType->Name);
  }

  SerializeName(mIndex);
}

//**************************************************************************************************
WidgetPath::WidgetPath()
{
}

//**************************************************************************************************
WidgetPath::WidgetPath(Widget* toWidget, RootWidget* fromRoot)
{
  while (toWidget != fromRoot)
  {
    ReturnIf(toWidget == nullptr, , "Encountered a null widget");
    ReturnIf(toWidget->mParent == nullptr,, "Encountered a widget without a parent");

    WidgetChildId& id = mPath.PushBack();
    id.mName = toWidget->mName;
    id.mType = ZilchVirtualTypeId(toWidget);
    id.mIndex = 0;

    forRange(Widget& sibling, toWidget->mParent->GetChildren())
    {
      if (&sibling == toWidget)
        break;

      if (sibling.mName == id.mName && ZilchVirtualTypeId(&sibling) == id.mType)
        ++id.mIndex;
    }

    toWidget = toWidget->mParent;
  }

  Reverse(mPath.Begin(), mPath.End());
}

//**************************************************************************************************
void WidgetPath::Serialize(Serializer& stream)
{
  SerializeName(mPath);
}

//**************************************************************************************************
Widget* WidgetPath::Resolve(RootWidget* root)
{
  Composite* composite = root;
  Widget* foundWidget = root;

  Array<WidgetChildId>::range path = mPath.All();
  forRange(WidgetChildId& childId, path)
  {
    size_t index = 0;
    forRange(Widget& childWidget, composite->GetChildren())
    {
      if (childWidget.GetName() == childId.mName && ZilchVirtualTypeId(&childWidget) == childId.mType)
      {
        if (index == childId.mIndex)
        {
          foundWidget = &childWidget;
          composite = foundWidget->GetSelfAsComposite();

          // If this widget isn't a composite, then we can't go any deeper into the tree
          if (composite == nullptr)
          {
            // If the path is also empty, we found the leaf widget!
            if (path.Empty())
              return foundWidget;
            // Otherwise we just hit a dead end
            else
              return nullptr;
          }
          break;
        }
        else
        {
          ++index;
        }
      }
    }
  }
  
  return foundWidget;
}

//--------------------------------------------------------------------------------- Unit Test Events
//**************************************************************************************************
ZilchDefineType(UnitTestEvent, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

//**************************************************************************************************
UnitTestEvent::~UnitTestEvent()
{
}

//**************************************************************************************************
ZilchDefineType(UnitTestBaseMouseEvent, builder, type)
{
}

//**************************************************************************************************
void UnitTestBaseMouseEvent::Serialize(Serializer& stream)
{
  SerializeName(mWidgetPath);
  SerializeName(mNormalizedWidgetOffset);
}

//**************************************************************************************************
ZilchDefineType(UnitTestMouseEvent, builder, type)
{
  ZilchBindConstructor();
}

//**************************************************************************************************
void UnitTestMouseEvent::Serialize(Serializer& stream)
{
  UnitTestBaseMouseEvent::Serialize(stream);
  mEvent.Serialize(stream);
}

//**************************************************************************************************
void UnitTestMouseEvent::Execute(UnitTestSystem* system)
{
  system->ExecuteBaseMouseEvent(this, &mEvent);
}

//**************************************************************************************************
ZilchDefineType(UnitTestMouseDropEvent, builder, type)
{
  ZilchBindConstructor();
}

//**************************************************************************************************
void UnitTestMouseDropEvent::Serialize(Serializer& stream)
{
  UnitTestBaseMouseEvent::Serialize(stream);
  mEvent.Serialize(stream);
}

//**************************************************************************************************
void UnitTestMouseDropEvent::Execute(UnitTestSystem* system)
{
  system->ExecuteBaseMouseEvent(this, &mEvent);
}

//**************************************************************************************************
ZilchDefineType(UnitTestKeyboardEvent, builder, type)
{
  ZilchBindConstructor();
}

//**************************************************************************************************
void UnitTestKeyboardEvent::Serialize(Serializer& stream)
{
  mEvent.Serialize(stream);
}

//**************************************************************************************************
void UnitTestKeyboardEvent::Execute(UnitTestSystem* system)
{
  system->GetMainWindow()->SendKeyboardEvent(mEvent);
}

//**************************************************************************************************
ZilchDefineType(UnitTestKeyboardTextEvent, builder, type)
{
  ZilchBindConstructor();
}

//**************************************************************************************************
void UnitTestKeyboardTextEvent::Serialize(Serializer& stream)
{
  mEvent.Serialize(stream);
}

//**************************************************************************************************
void UnitTestKeyboardTextEvent::Execute(UnitTestSystem* system)
{
  system->GetMainWindow()->DispatchEvent(mEvent.EventId, &mEvent);
}

//**************************************************************************************************
ZilchDefineType(UnitTestWindowEvent, builder, type)
{
  ZilchBindConstructor();
}

//**************************************************************************************************
void UnitTestWindowEvent::Serialize(Serializer& stream)
{
  mEvent.Serialize(stream);
}

//**************************************************************************************************
void UnitTestWindowEvent::Execute(UnitTestSystem* system)
{
  system->GetMainWindow()->DispatchEvent(mEvent.EventId, &mEvent);
}

//--------------------------------------------------------------------------------- Unit Test System
//**************************************************************************************************
ZilchDefineType(UnitTestSystem, builder, type)
{
}

//**************************************************************************************************
UnitTestSystem::UnitTestSystem() :
  mMode(UnitTestMode::Stopped),
  mFrameIndex(0),
  mEmulatedCursor(nullptr),
  mEventNumber(0)
{
}

//**************************************************************************************************
cstr UnitTestSystem::GetName()
{
  return "UnitTestSystem";
}

//**************************************************************************************************
void UnitTestSystem::Initialize(SystemInitializer& initializer)
{
}

//**************************************************************************************************
void UnitTestSystem::Update()
{
  if (mMode == UnitTestMode::Recording)
  {
    mFrames.PushBack(new UnitTestFrame());
  }
  else if (mMode == UnitTestMode::Playing)
  {
    if (mFrameIndex < mFrames.Size())
    {
      UnitTestFrame* frame = mFrames[mFrameIndex];
      
      forRange(UnitTestEvent* testEvent, frame->mEvents.All())
      {
        testEvent->Execute(this);
      }
    }
    ++mFrameIndex;
  }
}

//**************************************************************************************************
void UnitTestSystem::StartUnitTestRecording(StringParam zeroTestFile)
{
}

//**************************************************************************************************
void UnitTestSystem::PlayUnitTestRecording(StringParam zeroTestFile)
{
}

//**************************************************************************************************
void UnitTestSystem::StartUnitTestRecordingSubProcess()
{
  OsWindow* osWindow = GetMainWindow();
  if (osWindow == nullptr)
    return;

  mMode = UnitTestMode::Recording;

  // This should not be needed if these functions are called at the proper time
  mFrames.PushBack(new UnitTestFrame());

  // Mouse events
  ConnectThisTo(osWindow, Events::OsMouseDown, RecordMouseEvent);
  ConnectThisTo(osWindow, Events::OsMouseUp, RecordMouseEvent);
  ConnectThisTo(osWindow, Events::OsMouseMove, RecordMouseEvent);
  ConnectThisTo(osWindow, Events::OsMouseScroll, RecordMouseEvent);
  ConnectThisTo(osWindow, Events::OsMouseFileDrop, RecordMouseFileDropEvent);

  // Keyboard events
  ConnectThisTo(osWindow, Events::OsKeyDown, RecordKeyboardEvent);
  ConnectThisTo(osWindow, Events::OsKeyUp, RecordKeyboardEvent);
  ConnectThisTo(osWindow, Events::OsKeyRepeated, RecordKeyboardEvent);
  ConnectThisTo(osWindow, Events::OsKeyTyped, RecordKeyboardTextEvent);

  // Window events
  ConnectThisTo(osWindow, Events::OsFocusGained, RecordWindowEvent);
  ConnectThisTo(osWindow, Events::OsFocusLost, RecordWindowEvent);
}

//**************************************************************************************************
void UnitTestSystem::PlayUnitTestRecordingSubProcess()
{
  GetMainWindow();
  mMode = UnitTestMode::Playing;

  if (mEmulatedCursor == nullptr)
  {
    mEmulatedCursor = GetRootWidget()->CreateAttached<Widget>(cCursor);
    mEmulatedCursor->SetInteractive(false);
  }
}

//**************************************************************************************************
RootWidget* UnitTestSystem::GetRootWidget()
{
  InList<RootWidget>& rootWidgets = WidgetManager::GetInstance()->RootWidgets;
  ReturnIf(rootWidgets.Empty(),nullptr, "Cannot use the UnitTestSystem without a RootWidget");

  // We only work with the main window (the first root widget)
  RootWidget* root = &rootWidgets.Front();
  return root;
}

//**************************************************************************************************
OsWindow* UnitTestSystem::GetMainWindow()
{
  RootWidget* root = GetRootWidget();
  if (root == nullptr)
    return nullptr;

  return root->GetOsWindow();
}

//**************************************************************************************************
void UnitTestSystem::RecordMouseEvent(OsMouseEvent* event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestMouseEvent* testEvent = new UnitTestMouseEvent();
  testEvent->mEvent = *event;

  RecordBaseMouseEvent(testEvent, event);
}

//**************************************************************************************************
void UnitTestSystem::RecordMouseFileDropEvent(OsMouseDropEvent* event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestMouseDropEvent* testEvent = new UnitTestMouseDropEvent();
  testEvent->mEvent = *event;

  RecordEvent(testEvent);
}

//**************************************************************************************************
void UnitTestSystem::RecordKeyboardEvent(KeyboardEvent* event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestKeyboardEvent* testEvent = new UnitTestKeyboardEvent();
  testEvent->mEvent = *event;

  RecordEvent(testEvent);
}

//**************************************************************************************************
void UnitTestSystem::RecordKeyboardTextEvent(KeyboardTextEvent* event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestKeyboardTextEvent* testEvent = new UnitTestKeyboardTextEvent();
  testEvent->mEvent = *event;

  RecordEvent(testEvent);
}

//**************************************************************************************************
void UnitTestSystem::RecordWindowEvent(OsWindowEvent* event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestWindowEvent* testEvent = new UnitTestWindowEvent();
  testEvent->mEvent = *event;
  
  RecordEvent(testEvent);
}

//**************************************************************************************************
void UnitTestSystem::RecordBaseMouseEvent(UnitTestBaseMouseEvent* baseEvent, OsMouseEvent* event)
{
  Vec2 clientPos = ToVec2(event->ClientPosition);

  // Get the widget that the mouse is over
  RootWidget* root = GetRootWidget();
  Widget* overWidget = root->HitTest(clientPos, nullptr);
  if (overWidget == nullptr)
    overWidget = root;

  // Compute the normalized coordinates of the cursor within the widget
  Vec2 widgetClientTopLeft = overWidget->ToScreen(Vec2::cZero);
  baseEvent->mNormalizedWidgetOffset = (clientPos - widgetClientTopLeft) / overWidget->GetSize();

  // Compute the path to the widget from the root via WidgetPath constructor
  baseEvent->mWidgetPath = WidgetPath(overWidget, root);

  RecordEvent(baseEvent);
}

//**************************************************************************************************
void UnitTestSystem::ExecuteBaseMouseEvent(UnitTestBaseMouseEvent* baseEvent, OsMouseEvent* event)
{
  IntVec2 oldClientPosition = event->ClientPosition;

  // If we can find a widget via the widget path, then use the normalized coordinates to compute a new
  // position from inside the widget. This helps to ensure that, even if widgets move positions, mouse
  // events such as down/up still occur on the correct widgets.
  // If we cannot find a widget we use the original screen position and hope that it works!
  Widget* foundWidget = baseEvent->mWidgetPath.Resolve(GetRootWidget());
  if (foundWidget != nullptr)
  {
    Vec2 newClientPosition = foundWidget->ToScreen(Vec2::cZero) + foundWidget->GetSize() * baseEvent->mNormalizedWidgetOffset;
    event->ClientPosition = IntVec2((int)newClientPosition.x, (int)newClientPosition.y);
  }

  // Update the emulated cursor position (purely visual that cannot be interacted with)
  mEmulatedCursor->SetTranslation(Vec3(ToVec2(event->ClientPosition), 0));

  GetMainWindow()->DispatchEvent(event->EventId, event);

  event->ClientPosition = oldClientPosition;
}

//**************************************************************************************************
void UnitTestSystem::RecordEvent(UnitTestEvent* e)
{
  // Add it to our current frame
  mFrames.Back()->mEvents.PushBack(e);

  // Save out the event to our file. We want to append so that in case of a crash, we have already
  // saved out all input required to reproduce that crash
  String fileName = String::Format("Event%d.data", mEventNumber++);
  String file = FilePath::Combine(mRecordedEventsDirectory, fileName);
  TextSaver saver;
  Status status;
  saver.Open(status, file.c_str());

  ReturnIf(status.Failed(), , "Failed to save recorded event file");

  BoundType* type = ZilchVirtualTypeId(e);
  saver.StartPolymorphic(type);
  e->Serialize(saver);
  saver.EndPolymorphic();

  saver.Close();
}

//**************************************************************************************************
void UnitTestSystem::LoadRecordedEvents(StringParam directory)
{
  for(uint i = 0; ; ++i)
  {
    String fileName = String::Format("Event%d.data", i);
    String file = FilePath::Combine(mRecordedEventsDirectory, fileName);
    if (!FileExists(file))
      break;

    DataTreeLoader loader;
    Status status;
    loader.OpenFile(status, file);

    ReturnIf(status.Failed(), , "Failed to load recorded event file");

    PolymorphicNode eventNode;
    if(loader.GetPolymorphic(eventNode))
    {
      BoundType* type = MetaDatabase::FindType(eventNode.TypeName);
      UnitTestEvent* e = ZilchAllocate(UnitTestEvent, type);
      e->Serialize(loader);

      //mFrames.PushBack(e);

      loader.EndPolymorphic();
    }
  }
}

//**************************************************************************************************
UnitTestSystem* CreateUnitTestSystem()
{
  return new UnitTestSystem();
}
}// namespace Zero
