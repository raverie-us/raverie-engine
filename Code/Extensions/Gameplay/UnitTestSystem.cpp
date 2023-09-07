// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
DeclareEvent(UnitTestRecordFileSelected);
DeclareEvent(UnitTestPlayFileSelected);

DefineEvent(UnitTestRecordFileSelected);
DefineEvent(UnitTestPlayFileSelected);
} // namespace Events

// Widgets
WidgetChildId::WidgetChildId() : mType(nullptr), mIndex(0)
{
}

void WidgetChildId::Serialize(Serializer& stream)
{
  SerializeName(mName);

  if (stream.GetMode() == SerializerMode::Loading)
  {
    String typeName;
    stream.SerializeField("TypeName", typeName);
    mType = MetaDatabase::FindType(typeName);
  }
  else
  {
    if (mType)
      stream.SerializeField("TypeName", mType->Name);
  }

  SerializeName(mIndex);
}

WidgetPath::WidgetPath()
{
}

WidgetPath::WidgetPath(Widget* toWidget, RootWidget* fromRoot)
{
  while (toWidget != fromRoot)
  {
    ReturnIf(toWidget == nullptr, , "Encountered a null widget");
    ReturnIf(toWidget->mParent == nullptr, , "Encountered a widget without a parent");

    WidgetChildId& id = mPath.PushBack();
    id.mName = toWidget->mName;
    id.mType = ZilchVirtualTypeId(toWidget);
    id.mIndex = 0;

    forRange (Widget& sibling, toWidget->mParent->GetChildren())
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

void WidgetPath::Serialize(Serializer& stream)
{
  SerializeName(mPath);
}

Widget* WidgetPath::Resolve(RootWidget* root)
{
  Composite* composite = root;
  Widget* foundWidget = root;

  Array<WidgetChildId>::range path = mPath.All();
  forRange (WidgetChildId& childId, path)
  {
    size_t index = 0;
    forRange (Widget& childWidget, composite->GetChildren())
    {
      if (childWidget.GetName() == childId.mName && ZilchVirtualTypeId(&childWidget) == childId.mType)
      {
        if (index == childId.mIndex)
        {
          foundWidget = &childWidget;
          composite = foundWidget->GetSelfAsComposite();

          // If this widget isn't a composite, then we can't go any deeper into
          // the tree
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

// Unit Test Events
static const String cUnitTestRecordOption("unitTestRecord");
static const String cUnitTestPlayOption("unitTestPlay");
static const String cProjectBegin("ProjectBegin");
static const String cProjectEnd("ProjectEnd");
static const String cZeroProjWithoutDot("zeroproj");
static const String cZeroProjWithDot(".zeroproj");
static const IntVec2 cWindowSize = cMinimumMonitorSize;

ZilchDefineType(UnitTestEvent, builder, type)
{
  ZilchBindDestructor();
  type->HandleManager = ZilchManagerId(PointerManager);
}

UnitTestEvent::~UnitTestEvent()
{
}

ZilchDefineType(UnitTestEndFrameEvent, builder, type)
{
  ZilchBindConstructor();
  ZilchBindDestructor();
}

void UnitTestEndFrameEvent::Serialize(Serializer& stream)
{
}

void UnitTestEndFrameEvent::Execute(UnitTestSystem* system)
{
}

ZilchDefineType(UnitTestBaseMouseEvent, builder, type)
{
  ZilchBindDestructor();
}

void UnitTestBaseMouseEvent::Serialize(Serializer& stream)
{
  SerializeName(mWidgetPath);
  SerializeName(mNormalizedWidgetOffset);
}

ZilchDefineType(UnitTestMouseEvent, builder, type)
{
  ZilchBindConstructor();
  ZilchBindDestructor();
}

void UnitTestMouseEvent::Serialize(Serializer& stream)
{
  UnitTestBaseMouseEvent::Serialize(stream);
  mEvent.Serialize(stream);
}

void UnitTestMouseEvent::Execute(UnitTestSystem* system)
{
  mEvent.Window = system->GetMainWindow();
  IntVec2 oldClientPosition = mEvent.ClientPosition;
  system->ExecuteBaseMouseEvent(this, &mEvent);
  system->GetMainWindow()->SendMouseEvent(mEvent, true);
  mEvent.ClientPosition = oldClientPosition;
}

ZilchDefineType(UnitTestMouseDropEvent, builder, type)
{
  ZilchBindConstructor();
  ZilchBindDestructor();
}

void UnitTestMouseDropEvent::Serialize(Serializer& stream)
{
  UnitTestBaseMouseEvent::Serialize(stream);
  SerializeName(mFileIndex);
  mEvent.Serialize(stream);
}

void UnitTestMouseDropEvent::Execute(UnitTestSystem* system)
{
  // Update file paths to the cached files
  for (uint i = 0; i < mEvent.Files.Size(); ++i)
  {
    String& fileName = mEvent.Files[i];
    fileName = system->GetRecordedFile(mFileIndex, fileName);
  }

  mEvent.Window = system->GetMainWindow();
  IntVec2 oldClientPosition = mEvent.ClientPosition;
  system->ExecuteBaseMouseEvent(this, &mEvent);
  system->GetMainWindow()->SendMouseDropEvent(mEvent, true);
  mEvent.ClientPosition = oldClientPosition;
}

ZilchDefineType(UnitTestKeyboardEvent, builder, type)
{
  ZilchBindConstructor();
  ZilchBindDestructor();
}

void UnitTestKeyboardEvent::Serialize(Serializer& stream)
{
  mEvent.Serialize(stream);
}

void UnitTestKeyboardEvent::Execute(UnitTestSystem* system)
{
  mEvent.mKeyboard = Keyboard::GetInstance();
  system->GetMainWindow()->SendKeyboardEvent(mEvent, true);
}

ZilchDefineType(UnitTestKeyboardTextEvent, builder, type)
{
  ZilchBindConstructor();
  ZilchBindDestructor();
}

void UnitTestKeyboardTextEvent::Serialize(Serializer& stream)
{
  mEvent.Serialize(stream);
}

void UnitTestKeyboardTextEvent::Execute(UnitTestSystem* system)
{
  system->GetMainWindow()->SendKeyboardTextEvent(mEvent, true);
}

ZilchDefineType(UnitTestWindowEvent, builder, type)
{
  ZilchBindConstructor();
  ZilchBindDestructor();
}

void UnitTestWindowEvent::Serialize(Serializer& stream)
{
  mEvent.Serialize(stream);
}

void UnitTestWindowEvent::Execute(UnitTestSystem* system)
{
  system->GetMainWindow()->SendWindowEvent(mEvent, true);
}

// Unit Test System
ZilchDefineType(UnitTestSystem, builder, type)
{
}

UnitTestSystem::UnitTestSystem() :
    mEmulatedCursor(nullptr),
    mMode(UnitTestMode::Stopped),
    mEventIndex(0),
    mFilesIndex(0)
{
  ConnectThisTo(this, Events::UnitTestRecordFileSelected, OnUnitTestRecordFileSelected);
  ConnectThisTo(this, Events::UnitTestPlayFileSelected, OnUnitTestPlayFileSelected);
}

cstr UnitTestSystem::GetName()
{
  return "UnitTestSystem";
}

void UnitTestSystem::Initialize(SystemInitializer& initializer)
{
  bool unitTestSystemActive = false;

  // We can't initialize here because the root widget and main window have not
  // yet been created If the record or play modes were set, then we set a flag
  // to initialize on the first Update
  Environment* environment = Environment::GetInstance();
  if (!environment->GetParsedArgument(cUnitTestRecordOption).Empty())
  {
    unitTestSystemActive = true;
    mMode = UnitTestMode::StartRecording;
  }
  else if (!environment->GetParsedArgument(cUnitTestPlayOption).Empty())
  {
    unitTestSystemActive = true;
    mMode = UnitTestMode::StartPlaying;
  }

  // Is the unit test recording a replay or
  if (unitTestSystemActive)
  {
    // For determinism we lie about the frame-time that has passed to
    // the entire engine (it becomes a fixed value of 1/60)
    // Also force resource ids to be deterministic...
    gDeterministicMode = true;

    // Hook the OsShell update so we can process input events at the same time
    Z::gEngine->has(OsShell)->mOsShellHook = this;

    // When recording or playing we do not want the config to
    // be saved out since we launched with the 'safe' option
    MainConfig::sConfigCanSave = false;
  }
}

void UnitTestSystem::HookUpdate()
{
  // One time initialization logic for record and play modes
  if (mMode == UnitTestMode::StartRecording)
    SubProcessRecord();
  else if (mMode == UnitTestMode::StartPlaying)
    SubProcessPlay();

  if (mMode == UnitTestMode::Recording)
  {
    if (mEventIndex != 0)
      RecordEvent(new UnitTestEndFrameEvent());
  }
  else if (mMode == UnitTestMode::Playing)
  {
    while (mEventIndex < mEvents.Size())
    {
      UnitTestEvent* event = mEvents[mEventIndex];
      ++mEventIndex;

      if (ZilchVirtualTypeId(event) != ZilchTypeId(UnitTestEndFrameEvent))
        event->Execute(this);
      else
        break;
    }
  }
}

void UnitTestSystem::RecordToZeroTestFile()
{
  FileDialogConfig* config = FileDialogConfig::Create();
  config->EventName = Events::UnitTestRecordFileSelected;
  config->CallbackObject = this;
  config->Title = "Record Unit Test File";
  config->AddFilter("Unit Test (*.zerotest)", "*.zerotest");
  config->mDefaultSaveExtension = "zerotest";
  config->DefaultFileName = BuildString(Z::gEngine->GetProjectSettings()->ProjectName, ".zerotest");
  Z::gEngine->has(OsShell)->SaveFile(config);
}

void UnitTestSystem::OnUnitTestRecordFileSelected(OsFileSelection* event)
{
  if (event->Files.Empty())
    return;

  String filePath = event->Files.Front();
  RecordToZeroTestFile(filePath);
  Download(filePath);
}

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

  String newProjectFileName = BuildString(name, cZeroProjWithDot);
  String newBeginProjectFilePath = FilePath::Combine(beginFolder, newProjectFileName);
  String newEndProjectFilePath = FilePath::Combine(endFolder, newProjectFileName);

  // Rename the project files to the same name as the zerotest file
  MoveFile(newBeginProjectFilePath, oldBeginProjectFilePath);
  MoveFile(newEndProjectFilePath, oldEndProjectFilePath);

  // When recording, we always make our modifications to the end project
  // (therefore the end becomes the final result)
  info.mArguments = BuildString("-file \"", newEndProjectFilePath, "\" -safe -", cUnitTestRecordOption);

  Process process;
  process.Start(status, info);
  if (status.Failed())
  {
    DoNotifyError("UnitTestSystem", status.Message);
    return;
  }

  process.WaitForClose();

  Archive archive(ArchiveMode::Compressing);
  archive.ArchiveDirectory(status, directory);
  if (status.Failed())
  {
    DoNotifyError("UnitTestSystem", status.Message);
    return;
  }

  archive.WriteZipFile(zeroTestFile);
  DeleteDirectory(directory);
}

void UnitTestSystem::PlayFromZeroTestFile()
{
  FileDialogConfig* config = FileDialogConfig::Create();
  config->EventName = Events::UnitTestPlayFileSelected;
  config->CallbackObject = this;
  config->Title = "Play Unit Test File";
  config->AddFilter("Unit Test (*.zerotest)", "*.zerotest");
  Z::gEngine->has(OsShell)->OpenFile(config);
}

void UnitTestSystem::OnUnitTestPlayFileSelected(OsFileSelection* event)
{
  if (event->Files.Empty())
    return;

  PlayFromZeroTestFile(event->Files.Front());
}

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

  String beginProjectPath = FilePath::Combine(directory, cProjectBegin);
  String endProjectPath = FilePath::Combine(directory, cProjectEnd);

  // When playing back we always start from the beginning project (it should end
  // up the same as the end project)
  String projectFileName = BuildString(name, cZeroProjWithDot);
  String beginProjectFilePath = FilePath::Combine(beginProjectPath, projectFileName);
  String endProjectFilePath = FilePath::Combine(endProjectPath, projectFileName);

  info.mArguments = BuildString("-file \"", beginProjectFilePath, "\" -safe -", cUnitTestPlayOption);

  Process process;
  process.Start(status, info);
  if (status.Failed())
  {
    DoNotifyError("UnitTestSystem", status.Message);
    return;
  }

  process.WaitForClose();

  // Diff the directories
  DiffDirectories(beginProjectPath, endProjectPath);

  // The play should have finished, so delete the directory
  DeleteDirectory(directory);
}

void UnitTestSystem::SubProcessRecord()
{
  OsWindow* osWindow = SubProcessSetupWindow();

  // Let us capture input before everyone else
  osWindow->mOsInputHook = this;

  mMode = UnitTestMode::Recording;

  mPlaybackFile.Open(mRecordedEventsFile, FileMode::Append, FileAccessPattern::Sequential);
}

void UnitTestSystem::SubProcessPlay()
{
  OsWindow* osWindow = SubProcessSetupWindow();

  // Don't allow the user moving over the window to do anything
  osWindow->mBlockUserInput = true;

  LoadRecordedEvents();

  // During playback we always play as fast as possible (but we lie about frame
  // time)
  Cog* project = Z::gEngine->GetProjectSettings()->GetOwner();
  project->AddComponentByType(ZilchTypeId(FrameRateSettings));
  FrameRateSettings* frameRateSettings = project->has(FrameRateSettings);
  frameRateSettings->mVerticalSync = false;
  frameRateSettings->mLimitFrameRate = false;

  mMode = UnitTestMode::Playing;

  if (mEmulatedCursor == nullptr)
  {
    mEmulatedCursor = GetRootWidget()->CreateAttached<Widget>(cCursor);
    mEmulatedCursor->SetInteractive(false);
  }
}

OsWindow* UnitTestSystem::SubProcessSetupWindow()
{
  OsWindow* window = GetMainWindow();
  window->SetState(WindowState::Windowed);
  window->SetStyle((WindowStyleFlags::Enum)(window->GetStyle() & ~WindowStyleFlags::Resizable));
  window->SetClientSize(cWindowSize);

  IntVec2 monitorSize = Z::gEngine->has(OsShell)->GetPrimaryMonitorSize();
  IntVec2 centeredPosition = monitorSize / 2 - window->GetClientSize() / 2;
  window->SetMonitorClientPosition(centeredPosition);

  ProjectSettings* settings = Z::gEngine->GetProjectSettings();
  mRecordedEventsFile = FilePath::Combine(settings->ProjectFolder, "..", "Playback.data");
  mRecordedFilesDirectory = FilePath::Combine(settings->ProjectFolder, "..", "Files");
  return window;
}

void UnitTestSystem::DiffDirectories(StringParam dir1, StringParam dir2, StringParam diffProgram)
{
  HashSet<String> relativePaths1;
  HashSet<String> relativePaths2;

  EnumerateFiles(dir1, String(), &relativePaths1);
  EnumerateFiles(dir2, String(), &relativePaths2);

  forRange (StringParam relativePath, relativePaths1.All())
  {
    // If both directories have this path...
    if (relativePaths2.Contains(relativePath))
    {
      relativePaths2.Erase(relativePath);

      // Don't diff the zeroproj file because the verison will
      // often be different (and frame rate settings may change)
      if (FilePath::GetExtension(relativePath) == cZeroProjWithoutDot)
        continue;

      String path1 = FilePath::Combine(dir1, relativePath);
      String path2 = FilePath::Combine(dir2, relativePath);

      Status status;

      String hash1 = Zilch::Sha1Builder::GetHashStringFromFile(status, path1);
      if (status.Failed())
      {
        DoNotifyWarning("UnitTestSystem", String::Format("Failed to open file: %s", status.Message.c_str()));
        continue;
      }

      String hash2 = Zilch::Sha1Builder::GetHashStringFromFile(status, path2);
      if (status.Failed())
      {
        DoNotifyWarning("UnitTestSystem", String::Format("Failed to open file: %s", status.Message.c_str()));
        continue;
      }

      if (hash1 != hash2)
      {
        DoNotifyWarning("UnitTestSystem",
                        String::Format("Diff failed because file %s did not match", relativePath.c_str()));
        if (!diffProgram.Empty())
        {
          ProcessStartInfo info;
          info.mApplicationName = diffProgram;
          info.mArguments = BuildString("\"", path1.c_str(), "\" \"", path2.c_str(), "\"");

          Process process;
          process.Start(status, info);

          if (status.Failed())
          {
            DoNotifyWarning("UnitTestSystem", String::Format("Failed run diff process: %s", status.Message.c_str()));
            continue;
          }
        }
      }
    }
    else
    {
      DoNotifyWarning(
          "UnitTestSystem",
          String::Format("Diff failed because file %s did not exist in both directories", relativePath.c_str()));
    }
  }

  forRange (StringParam relativePath, relativePaths2.All())
  {
    DoNotifyWarning(
        "UnitTestSystem",
        String::Format("Diff failed because file %s did not exist in both directories", relativePath.c_str()));
  }
}

void UnitTestSystem::EnumerateFiles(StringParam directory,
                                    StringParam relativeParentPath,
                                    HashSet<String>* relativePaths)
{
  FileRange files(directory);
  for (; !files.Empty(); files.PopFront())
  {
    String fileName = files.Front();
    String relativePath = FilePath::Combine(relativeParentPath, fileName);
    String fullPath = FilePath::Combine(directory, fileName);

    if (DirectoryExists(fullPath))
      EnumerateFiles(fullPath, relativePath, relativePaths);
    else
      relativePaths->Insert(relativePath);
  }
}

RootWidget* UnitTestSystem::GetRootWidget()
{
  InList<RootWidget>& rootWidgets = WidgetManager::GetInstance()->RootWidgets;
  ReturnIf(rootWidgets.Empty(), nullptr, "Cannot use the UnitTestSystem without a RootWidget");

  // We only work with the main window (the first root widget)
  RootWidget* root = &rootWidgets.Front();
  return root;
}

OsWindow* UnitTestSystem::GetMainWindow()
{
  RootWidget* root = GetRootWidget();
  if (root == nullptr)
    return nullptr;

  return root->GetOsWindow();
}

void UnitTestSystem::HookKeyboardEvent(KeyboardEvent& event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestKeyboardEvent* testEvent = new UnitTestKeyboardEvent();
  testEvent->mEvent = event;

  RecordEvent(testEvent);
}

void UnitTestSystem::HookKeyboardTextEvent(KeyboardTextEvent& event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestKeyboardTextEvent* testEvent = new UnitTestKeyboardTextEvent();
  testEvent->mEvent = event;

  RecordEvent(testEvent);
}

void UnitTestSystem::HookMouseEvent(OsMouseEvent& event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestMouseEvent* testEvent = new UnitTestMouseEvent();
  testEvent->mEvent = event;

  RecordBaseMouseEvent(testEvent, &event);
}

void UnitTestSystem::HookMouseDropEvent(OsMouseDropEvent& event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestMouseDropEvent* testEvent = new UnitTestMouseDropEvent();
  testEvent->mEvent = event;

  testEvent->mFileIndex = mFilesIndex;

  forRange (String& file, testEvent->mEvent.Files)
  {
    RecordFile(file);

    // Store the exact file names so we have the same order when we load them
    // back in
    file = FilePath::GetFileName(file);
  }

  ++mFilesIndex;
  RecordBaseMouseEvent(testEvent, &event);
}

void UnitTestSystem::HookWindowEvent(OsWindowEvent& event)
{
  if (mMode != UnitTestMode::Recording)
    return;

  UnitTestWindowEvent* testEvent = new UnitTestWindowEvent();
  testEvent->mEvent = event;

  RecordEvent(testEvent);
}

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
  Vec2 size = overWidget->GetSize();
  if (size.x <= 0 || size.y <= 0)
    baseEvent->mNormalizedWidgetOffset = Vec2(0, 0);
  else
    baseEvent->mNormalizedWidgetOffset = (clientPos - widgetClientTopLeft) / size;

  // Compute the path to the widget from the root via WidgetPath constructor
  baseEvent->mWidgetPath = WidgetPath(overWidget, root);

  RecordEvent(baseEvent);
}

void UnitTestSystem::ExecuteBaseMouseEvent(UnitTestBaseMouseEvent* baseEvent, OsMouseEvent* event)
{
  // If we can find a widget via the widget path, then use the normalized
  // coordinates to compute a new position from inside the widget. This helps to
  // ensure that, even if widgets move positions, mouse events such as down/up
  // still occur on the correct widgets. If we cannot find a widget we use the
  // original screen position and hope that it works!
  Widget* foundWidget = baseEvent->mWidgetPath.Resolve(GetRootWidget());
  if (foundWidget != nullptr)
  {
    Vec2 newClientPosition =
        foundWidget->ToScreen(Vec2::cZero) + foundWidget->GetSize() * baseEvent->mNormalizedWidgetOffset;
    event->ClientPosition = IntVec2((int)newClientPosition.x, (int)newClientPosition.y);
  }

  // Update the emulated cursor position (purely visual that cannot be
  // interacted with)
  mEmulatedCursor->SetTranslation(Vec3(ToVec2(event->ClientPosition), 0));
  mEmulatedCursor->SetVisible(true);
  mEmulatedCursor->SetActive(true);
  mEmulatedCursor->MoveToFront();
}

void UnitTestSystem::RecordFile(StringParam sourceFile)
{
  String fileName = FilePath::GetFileName(sourceFile);

  String subDirectory = String::Format("Files%d", mFilesIndex);
  String destinationDirectory = FilePath::Combine(mRecordedFilesDirectory, subDirectory);
  String destinationFile = FilePath::Combine(destinationDirectory, fileName);

  CreateDirectoryAndParents(destinationDirectory);
  CopyFile(destinationFile, sourceFile);
}

String UnitTestSystem::GetRecordedFile(uint fileIndex, StringParam fileName)
{
  String subDirectory = String::Format("Files%d", fileIndex);
  String destinationDirectory = FilePath::Combine(mRecordedFilesDirectory, subDirectory);
  return FilePath::Combine(destinationDirectory, fileName);
}

void UnitTestSystem::RecordEvent(UnitTestEvent* e)
{
  FileMode::Enum fileMode = FileMode::Append;

  if (mEventIndex == 0)
    fileMode = FileMode::Write;

  // Add it to our current frame
  mEvents.PushBack(e);
  ++mEventIndex;

  // Save out the event to our file. We want to append so that in case of a
  // crash, we have already saved out all input required to reproduce that crash
  TextSaver saver;
  saver.OpenBuffer(DataVersion::Current, fileMode);

  BoundType* type = ZilchVirtualTypeId(e);
  saver.StartPolymorphic(type);
  e->Serialize(saver);
  saver.EndPolymorphic();

  forRange (const ByteBuffer::Block& block, saver.mStream.Blocks())
  {
    mPlaybackFile.Write(block.Data, block.Size);
  }

  mPlaybackFile.Flush();
}

void UnitTestSystem::LoadRecordedEvents()
{
  DataTreeLoader loader;
  Status status;
  loader.OpenFile(status, mRecordedEventsFile);

  ReturnIf(status.Failed(), , "Failed to load recorded event file");

  PolymorphicNode eventNode;
  while (loader.GetPolymorphic(eventNode))
  {
    BoundType* type = MetaDatabase::FindType(eventNode.TypeName);
    UnitTestEvent* e = ZilchAllocate(UnitTestEvent, type);
    e->Serialize(loader);

    mEvents.PushBack(e);

    loader.EndPolymorphic();
  }
}

UnitTestSystem* CreateUnitTestSystem()
{
  return new UnitTestSystem();
}

} // namespace Zero
