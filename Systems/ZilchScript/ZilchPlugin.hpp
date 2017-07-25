///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ZilchScript.hpp"

namespace Zero
{
//-------------------------------------------------------------------ZilchPluginSource
class BackgroundTask;
class ZilchPluginConfig;
class UpdateEvent;

class ZilchPluginSource : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ZilchPluginSource();
  ~ZilchPluginSource();

  // Resource interface
  void Serialize(Serializer& stream) override;
  void EditorInitialize();

  void OnEngineUpdate(UpdateEvent* event);

  String GetContentDirectory();
  String GetCodeDirectory();
  String GetVersionsDirectory();
  String GetCurrentVersionDirectory();

  // Copies the library and header files to the plugin shared directory
  // We pretty much litter this function in every call to open the plugin (ide, folder, upon loading, etc)
  void ForceCopyPluginDependencies();
  void CopyPluginDependencies();
  void CopyPluginDependenciesOnce();
  void WriteCurrentVersionFile();

  void OpenDirectory();
  void OpenIde();
  void CompileDebug();
  void CompileRelease();
  void CompileConfiguration(StringParam configuration);
  void Clean();
  void InstallIdeTools();

  bool CheckIdeAndInformUser();
  bool IsIdeInstalled();
  bool IsIdeToolInstalled();
  bool ShouldInstallIdeTools();
  void MarkAttemptedIdeToolsInstAll();

  ZilchPluginConfig* GetConfig();

  // Internals
  BackgroundTask* mCompileTask;

  // This counter starts at 0, and if the user attempts to open the IDE
  // without installing the IDE tools, then we launch the IDE tools installer
  // We have to wait for that installer to close, however the installer
  // seems to run itself multiple times (perhaps elevating permissions)
  // So we need a counter to be sure the installer is finished
  static const int InstallCounter = 20;
  int mOpenIdeAfterToolsInstallCounter;
};

//-------------------------------------------------------------------ZilchPluginSourceLoader
class ZilchPluginSourceLoader : public ResourceLoader
{
public:
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override;
};

//-------------------------------------------------------------------ZilchPluginSourceManager
class ZilchPluginSourceManager : public ResourceManager
{
public:
  DeclareResourceManager(ZilchPluginSourceManager, ZilchPluginSource);

  ZilchPluginSourceManager(BoundType* resourceType);
  ~ZilchPluginSourceManager();

  // ResourceManager Interface
  void ValidateName(Status& status, StringParam name) override;
  
  void OnResourceEvent(ResourceEvent* event);

  bool IsCompilingPlugins();

  // Internal
  // How many plugins we're currently compiling
  size_t mCompilingPluginCount;
};

//-------------------------------------------------------------------ZilchPluginLibrary
class ZilchPluginLibrary : public ZilchLibraryResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ZilchPluginLibrary();
  ~ZilchPluginLibrary();

  void AddToProject(Project& project) override;
  
  String SharedLibraryPath;
  String GetSharedLibraryPath();
};

//-------------------------------------------------------------------ZilchPluginLibraryLoader
class ZilchPluginLibraryLoader : public ResourceLoader
{
public:
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
};

//-------------------------------------------------------------------ZilchPluginLibraryManager
class ZilchPluginLibraryManager : public ResourceManager
{
public:
  DeclareResourceManager(ZilchPluginLibraryManager, ZilchPluginLibrary);

  ZilchPluginLibraryManager(BoundType* resourceType);
  ~ZilchPluginLibraryManager();
  
  void OnResourceEvent(ResourceEvent* event);
};

}//namespace Zero
