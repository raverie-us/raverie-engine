///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
DeclareEvent(UnitTestRecordFileSelected);
DeclareEvent(UnitTestPlayFileSelected);

DefineEvent(UnitTestRecordFileSelected);
DefineEvent(UnitTestPlayFileSelected);
}

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
static const String cUnitTestRecordOption("unitTestRecord");
static const String cUnitTestPlayOption("unitTestPlay");
static const String cProjectBegin("ProjectBegin");
static const String cProjectEnd("ProjectEnd");
static const IntVec2 cWindowSize(1024, 768);

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
  IntVec2 oldClientPosition = mEvent.ClientPosition;
  system->ExecuteBaseMouseEvent(this, &mEvent);
  system->GetMainWindow()->SendMouseEvent(mEvent);
  mEvent.ClientPosition = oldClientPosition;
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
  IntVec2 oldClientPosition = mEvent.ClientPosition;
  system->ExecuteBaseMouseEvent(this, &mEvent);
  system->GetMainWindow()->SendMouseDropEvent(mEvent);
  mEvent.ClientPosition = oldClientPosition;
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
  system->GetMainWindow()->SendKeyboardTextEvent(mEvent);
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
  system->GetMainWindow()->SendWindowEvent(mEvent);
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
  ConnectThisTo(this, Events::UnitTestRecordFileSelected, OnUnitTestRecordFileSelected);
  ConnectThisTo(this, Events::UnitTestPlayFileSelected, OnUnitTestPlayFileSelected);
}

//**************************************************************************************************
cstr UnitTestSystem::GetName()
{
  return "UnitTestSystem";
}

//**************************************************************************************************
void UnitTestSystem::Initialize(SystemInitializer& initializer)
{
  // We can't initialize here because the root widget and main window have not yet been created
  // If the record or play modes were set, then we set a flag to initialize on the first Update
  Environment* environment = Environment::GetInstance();
  if (!environment->GetParsedArgument(cUnitTestRecordOption).Empty())
  {
    mMode = UnitTestMode::StartRecording;
  }
  else if (!environment->GetParsedArgument(cUnitTestPlayOption).Empty())
  {
    mMode = UnitTestMode::StartPlaying;
  }
}

//**************************************************************************************************
void UnitTestSystem::Update()
{
  // One time initialization logic for record and play modes
  if (mMode == UnitTestMode::StartRecording)
    SubProcessRecord();
  else if (mMode == UnitTestMode::StartPlaying)
    SubProcessPlay();

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
void UnitTestSystem::RecordToZeroTestFile()
{
  FileDialogConfig config;
  config.EventName = Events::UnitTestRecordFileSelected;
  config.CallbackObject = this;
  config.Title = "Record Unit Test File";
  config.AddFilter("Zero Unit Test", "*.zerotest");
  Z::gEngine->has(OsShell)->OpenFile(config);
}

//**************************************************************************************************
void UnitTestSystem::OnUnitTestRecordFileSelected(OsFileSelection* event)
{
  if (event->Files.Empty())
    return;

  RecordToZeroTestFile(event->Files.Front());
}

//**************************************************************************************************
void UnitTestSystem::RecordToZeroTestFile(StringParam zeroTestFile)
{
  String name = FilePath::GetFileNameWithoutExtension(zeroTestFile);
  String directory = FilePath::Combine(FilePath::GetDirectoryPath(zeroTestFile), name);
  if (DirectoryExists(directory))
  {
    DoNotifyError("UnitTestSystem", "There must not be a directory of the same name next to the test file");
    return;
  }

  // In case the file already exists, delete it
  if (FileExists(zeroTestFile))
    DeleteFile(zeroTestFile);

  Z::gEditor->SaveAll(false);

  // We create two directories so we can compare the beginning and the end
  String beginFolder = FilePath::Combine(directory, cProjectBegin);
  String endFolder = FilePath::Combine(directory, cProjectEnd);

  ProjectSettings* settings = Z::gEngine->GetProjectSettings();
  String projectFolder = settings->ProjectFolder;

  CopyFolderContents(beginFolder, projectFolder);
  CopyFolderContents(endFolder, projectFolder);

  Status status;
  ProcessStartInfo info;

  info.mApplicationName = GetApplication();

  String oldProjectFileName = FilePath::GetFileName(settings->ProjectFile);
  String oldBeginProjectFilePath = FilePath::Combine(beginFolder, oldProjectFileName);
  String oldEndProjectFilePath = FilePath::Combine(endFolder, oldProjectFileName);

  String newProjectFileName = BuildString(name, ".zeroproj");
  String newBeginProjectFilePath = FilePath::Combine(beginFolder, newProjectFileName);
  String newEndProjectFilePath = FilePath::Combine(endFolder, newProjectFileName);

  // Rename the project files to the same name as the zerotest file
  MoveFile(newBeginProjectFilePath, oldBeginProjectFilePath);
  MoveFile(newEndProjectFilePath, oldEndProjectFilePath);

  // When recording, we always make our modifications to the end project (therefore the end becomes the final result)
  info.mArguments = BuildString("-file \"", newEndProjectFilePath, "\" -safe -", cUnitTestRecordOption);

  Process process;
  process.Start(status, info);
}

//**************************************************************************************************
void UnitTestSystem::PlayFromZeroTestFile()
{
  FileDialogConfig config;
  config.EventName = Events::UnitTestPlayFileSelected;
  config.CallbackObject = this;
  config.Title = "Play Unit Test File";
  config.AddFilter("Zero Unit Test", "*.zerotest");
  Z::gEngine->has(OsShell)->OpenFile(config);
}

//**************************************************************************************************
void UnitTestSystem::OnUnitTestPlayFileSelected(OsFileSelection* event)
{
  if (event->Files.Empty())
    return;

  RecordToZeroTestFile(event->Files.Front());
}

//**************************************************************************************************
void UnitTestSystem::PlayFromZeroTestFile(StringParam zeroTestFile)
{
  String name = FilePath::GetFileNameWithoutExtension(zeroTestFile);
  String directory = FilePath::Combine(FilePath::GetDirectoryPath(zeroTestFile), name);
  if (DirectoryExists(directory))
  {
    DoNotifyError("UnitTestSystem", "There must not be a directory of the same name next to the test file");
    return;
  }

  if (!FileExists(zeroTestFile))
  {
    DoNotifyError("UnitTestSystem", "The specified file does not exist");
    return;
  }

  Archive archive(ArchiveMode::Decompressing);
  archive.ReadZipFile(ArchiveReadFlags::All, zeroTestFile);
  archive.ExportToDirectory(ArchiveExportMode::Overwrite, directory);

  Status status;
  ProcessStartInfo info;

  info.mApplicationName = GetApplication();

  // When playing back we always start from the beginning project (it should end up the same as the end project)
  String projectFileName = BuildString(name, ".zeroproj");
  String projectFilePath = FilePath::Combine(directory, cProjectBegin, projectFileName);
  info.mArguments = BuildString("-file \"", projectFilePath, "\" -safe -", cUnitTestPlayOption);

  Process process;
  process.Start(status, info);
}

//**************************************************************************************************
void UnitTestSystem::SubProcessRecord()
{
  OsWindow* osWindow = SubProcessSetupWindow();

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
void UnitTestSystem::SubProcessPlay()
{
  SubProcessSetupWindow();

  mMode = UnitTestMode::Playing;

  if (mEmulatedCursor == nullptr)
  {
    mEmulatedCursor = GetRootWidget()->CreateAttached<Widget>(cCursor);
    mEmulatedCursor->SetInteractive(false);
  }
}

//**************************************************************************************************
OsWindow* UnitTestSystem::SubProcessSetupWindow()
{
  OsWindow* window = GetMainWindow();
  window->SetState(WindowState::Windowed);
  window->SetStyle((WindowStyleFlags::Enum)(window->GetStyle() & ~WindowStyleFlags::Resizable));
  window->SetClientSize(cWindowSize);
  return window;
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
