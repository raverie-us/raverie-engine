///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Trevor Sudberg
/// Copyright 2010-2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class OsFileSelection;
class File;
class Thread;

void ExportContentFolders(Cog* projectCog);

class Exporter : public ExplicitSingleton<Exporter, EventObject>
{
public:
  typedef Exporter ZilchSelf;

  Exporter();
  void OnFilesSelected(OsFileSelection* file);
  void ExportGameProject(Cog* project);
  void ExportAndPlay(Cog* project);

  void DoExport();
private:
  void BeginExport();
  Cog* mProjectCog;
  String mOutputFile;
  bool mPlay;
};

};

