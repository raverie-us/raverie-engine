// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
class OsFileSelection;
class File;
class Thread;
class Exporter;

namespace ExportUtility
{
typedef void (*FileCallback)(
    StringParam fullPath, StringParam relativePath, StringParam fileName, void* userData, float progressPercent);
HashSet<String>& GetExcludedFiles();
void AddFilesHelper(StringParam directory,
                    StringParam relativePathFromStart,
                    HashSet<String>& additionalFileExcludes,
                    FileCallback callback,
                    void* userData);
void ExportContentFolders(Cog* projectCog);
void AddFiles(StringParam directory, HashSet<String>& additionalFileExcludes, FileCallback callback, void* userData);
void ArchiveLibraryOutput(Archive& archive, ContentLibrary* library);
void ArchiveLibraryOutput(Archive& archive, StringParam libraryName);
void CopyLibraryOut(StringParam outputDirectory, ContentLibrary* library, bool skipTemplates = true);
void CopyLibraryOut(StringParam outputDirectory, StringParam name, bool skipTemplates = true);
void RelativeCopyFile(StringParam dest, StringParam source, StringParam filename);
void ArchiveFileCallback(
    StringParam fullPath, StringParam relativePath, StringParam fileName, void* userData, float progressPercent);
void CopyFileCallback(StringParam fullPath, StringParam relativePath, StringParam fileName, void* userData);
} // namespace ExportUtility

// ExportTarget
class ExportTarget
{
public:
  ExportTarget(Exporter* exporter, String targetName) : mExporter(exporter), mTargetName(targetName){};

  virtual HashSet<String>& GetAdditionalExcludedFiles()
  {
    static HashSet<String> files;
    return files;
  };

  virtual void ExportApplication() = 0;
  virtual void ExportContentFolders(Cog* projectCog) = 0;
  virtual void CopyInstallerSetupFile(StringParam dest, StringParam source, StringParam projectName, Guid guid) = 0;

  Exporter* mExporter;
  String mTargetName;
};

// WindowsExportTarget
class WindowsExportTarget : public ExportTarget
{
public:
  WindowsExportTarget(Exporter* exporter, String targetName);

  HashSet<String>& GetAdditionalExcludedFiles() override;
  void ExportApplication() override;
  void ExportContentFolders(Cog* projectCog) override;
  void CopyInstallerSetupFile(StringParam dest, StringParam source, StringParam projectName, Guid guid) override;
};

// EmscriptenExportTarget
class EmscriptenExportTarget : public ExportTarget
{
public:
  EmscriptenExportTarget(Exporter* exporter, String targetName);

  HashSet<String>& GetAdditionalExcludedFiles() override;
  void ExportApplication() override;
  void ExportContentFolders(Cog* projectCog) override;
  void CopyInstallerSetupFile(StringParam dest, StringParam source, StringParam projectName, Guid guid) override;
};

// ExportTargetEntry
struct ExportTargetSource;
struct ExportTargetEntry
{
  ExportTargetEntry(String target) : TargetName(target), Export(false){};

  String TargetName;
  bool Export;
};

// ExportTargetList
struct ExportTargetList
{
  ExportTargetList();
  ~ExportTargetList();

  void AddEntry(ExportTargetEntry* entry);
  void SetActiveTargets(HashSet<String>& activeTargets);
  HashSet<String> GetActiveTargets();

  HashMap<String, ExportTargetEntry*> Entries;
  Array<ExportTargetEntry*> SortedEntries;
};

class ExportUI : public Composite
{
public:
  typedef ExportUI ZilchSelf;

  ExportUI(Composite* parent);
  ~ExportUI();

  static void OpenExportWindow();

  void SetTreeFormatting();

  // Event handlers
  void OnExportApplication(Event* e);
  void OnExportContentFolder(Event* e);
  void OnCancel(Event* e);
  void OnSelectPath(Event* e);
  void OnFolderSelected(OsFileSelection* e);

  void SetAvailableTargets(HashSet<String>& targets);
  void SetActiveTargets(HashSet<String>& targets);
  void SaveActiveTargets(HashSet<String>& targets);

  TextBox* mApplicationName;
  TextBox* mExportPath;

  TreeView* mTreeView;
  ExportTargetList mTargetList;
  ExportTargetSource* mSource;
};

class Exporter : public ExplicitSingleton<Exporter, EventObject>
{
public:
  typedef Exporter ZilchSelf;

  Exporter();
  void ExportGameProject(Cog* project);
  void ExportAndPlay(Cog* project);

  void ExportApplication(HashSet<String> exportTargets);
  void ExportContent(HashSet<String> exportTargets);

  void CopyContent(Status& status, String outputDirectory, ExportTarget* target);
  void UpdateIcon(ProjectSettings* project, ExecutableResourceUpdater& updater);

  void SaveAndBuildContent();

  // Project export settings
  Cog* mProjectCog;
  bool mPlay;

  // Output information
  String mOutputDirectory;
  String mApplicationName;

  // Export target settings
  HashSet<String> mDefaultTargets;
  HashMap<String, ExportTarget*> mExportTargets;
};

}; // namespace Zero
