// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Zero
{

void OpenDocumentation()
{
  ImportOpenUrl(Urls::cUserOnlineDocs);
}

void ExitEditor()
{
  if (Z::gWidgetManager->RootWidgets.Empty())
    return;

  OsWindowEvent event;
  OsWindow::sInstance->DispatchEvent(Events::OsClose, &event);
}

void ShowTextWindow(StringParam windowTitle, StringParam windowText)
{
  Window* window = new Window(Z::gEditor);
  window->SetTitle(windowTitle);

  window->SetTranslation(Vec3(0, -500, 0));
  MultiLineText* textBox = new MultiLineText(window);
  textBox->SetText(windowText);

  window->SizeToContents();
  CenterToWindow(Z::gEditor, window, true);
}

void ShowAbout()
{
  String text = GetApplicationName() + "\n" + GetBuildVersionName() + "\n";
  ShowTextWindow("About", text);
}

DeclareEnum3(VersionStatus, Unknown, UpToDate, NewVersionAvailable);

VersionStatus::Type GlobalVersionStatus = VersionStatus::Unknown;

void BuildVersion()
{
  String buildVersionString = String::Format("BuildVersion: %s", GetBuildVersionName().c_str());
  ZPrintFilter(Filter::DefaultFilter, "%s\n", buildVersionString.c_str());
  OsShell* platform = Z::gEngine->has(OsShell);
}

void WriteBuildInfo()
{
  String sourceDir = Z::gEngine->GetConfigCog()->has(MainConfig)->SourceDirectory;
  Environment* environment = Environment::GetInstance();
  String filePath = environment->GetParsedArgument("WriteBuildInfo");
  if (filePath.Empty() || filePath == "true")
    filePath = FilePath::CombineWithExtension(sourceDir, "Meta", ".data");

  StringBuilder builder;
  builder.AppendFormat("MajorVersion %d\n", GetMajorVersion());
  builder.AppendFormat("MinorVersion %d\n", GetMinorVersion());
  builder.AppendFormat("PatchVersion %d\n", GetPatchVersion());
  builder.AppendFormat("RevisionId %d\n", GetRevisionNumber());
  builder.AppendFormat("ShortChangeSet \"%s\"\n", GetShortChangeSetString());
  builder.AppendFormat("ChangeSet \"%s\"\n", GetChangeSetString());
  builder.AppendFormat("ChangeSetDate \"%s\"\n", GetChangeSetDateString());
  builder.AppendFormat("BuildId \"%s\"\n", GetBuildIdString().c_str());

  String result = builder.ToString();
  WriteStringRangeToFile(filePath, result);

  ZPrint("Writing build info to '%s'\n", filePath.c_str());
  ZPrint("File Contents: %s\n", result.c_str());

  Z::gEngine->Terminate();
}

void OpenTestWidgetsCommand()
{
  OpenTestWidgets(Z::gEditor);
}

void SortAndPrintMetaTypeList(StringBuilder& builder, Array<String>& names, cstr category)
{
  if (names.Empty())
    return;

  Sort(names.All());

  builder.Append(category);
  builder.Append(":\n");
  forRange (String& name, names.All())
  {
    builder.Append("  ");
    builder.Append(name);
    builder.Append("\n");
  }
  builder.Append("\n");
}

void CopyLibraryOut(StringParam outputDirectory, ContentLibrary* library, bool skipTemplates)
{
  String libraryPath = library->GetOutputPath();
  if (!DirectoryExists(libraryPath))
  {
    ZPrint("Skipped copying library output because it was not built %s\n", library->Name.c_str());
    return;
  }

  String libraryOutputPath = FilePath::Combine(outputDirectory, library->Name);

  CreateDirectoryAndParents(libraryOutputPath);

  // Copy the .pack file
  String packFile = BuildString(library->Name, ".pack");
  String packFileSource = FilePath::Combine(libraryPath, packFile);
  if (FileExists(packFileSource))
  {
    String packFileDestination = FilePath::Combine(libraryOutputPath, packFile);
    CopyFile(packFileDestination, packFileSource);
  }

  BoundType* zilchDocumentType = ZilchTypeId(ZilchDocumentResource);

  int itemsDone = 0;
  float librarySize = (float)library->GetContentItems().Size();

  forRange (ContentItem* contentItem, library->GetContentItems())
  {
    ++itemsDone;
    bool isTemplate = contentItem->has(ResourceTemplate);

    // Copy each generated Resource
    ResourceListing listing;
    contentItem->BuildListing(listing);
    forRange (ResourceEntry& entry, listing.All())
    {
      // Skip zilch Resource Templates
      if (isTemplate && skipTemplates)
      {
        BoundType* resourceType = MetaDatabase::FindType(entry.Type);

        // Skip zilch resource types
        if (resourceType->IsA(zilchDocumentType))
        {
          continue;
        }
      }

      String fileName = entry.Location;
      String source = FilePath::Combine(libraryPath, fileName);
      if (!FileExists(source))
        continue;

      String destination = FilePath::Combine(libraryOutputPath, fileName);
      CopyFile(destination, source);
      Z::gEngine->LoadingUpdate("Copying", fileName, "", ProgressType::Normal, float(itemsDone) / librarySize);
    }
  }
}

void CopyPrebuiltContent()
{
  ZPrint("Copying prebuilt content...\n");
  // Save all resources and build them so the
  // output directory is up to date
  if (Z::gEditor)
    Z::gEditor->SaveAll(false, false);
  // We copy all libraries (including Fallback) because we only expect this to be run by the install steps
  const String outputDirectory = Z::gContentSystem->PrebuiltContentPath;
  forRange (ContentLibrary* library, Z::gContentSystem->Libraries.Values())
  {
    ZPrint("  Copying %s\n", library->Name.c_str());
    CopyLibraryOut(outputDirectory, library, false);
  }
  Download(outputDirectory);
}

void BindAppCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("About", BindCommandFunction(ShowAbout), true);

  commands->AddCommand("Exit", BindCommandFunction(ExitEditor));

  commands->AddCommand("BuildVersion", BindCommandFunction(BuildVersion), true);
  commands->AddCommand("WriteBuildInfo", BindCommandFunction(WriteBuildInfo));

  if (DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig))
  {
    commands->AddCommand("OpenTestWidgets", BindCommandFunction(OpenTestWidgetsCommand));
  }
  commands->AddCommand("Documentation", BindCommandFunction(OpenDocumentation), true);

  commands->AddCommand("CopyPrebuiltContent", BindCommandFunction(CopyPrebuiltContent));
}

} // namespace Zero
