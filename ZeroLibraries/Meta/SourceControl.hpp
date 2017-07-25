///////////////////////////////////////////////////////////////////////////////
///
/// \file SourceControl.hpp
///  Source Control Interface
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Utility class for ContentHistory. Stores the info for an
/// individual revision within the entire content history.
class Revision : public IZilchObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String Date;
  String User;
  uint Number;
  String ChangeSet;
  String Summary;

  void SetDefaults()
  {
  }
};


/// Source Control interface for when the editor makes changes to files under source control.
/// Not meant to be a complete source control management interface just to make the editor
/// cleanly update state that can not be easily detected (Add/Remove/Rename).
/// Also used to access revisions and other data for display.
class SourceControl : public Object
{
public:
  /// Add a file to source control. (full path)
  virtual void Add(Status& status, StringParam filePath)=0;

  /// Remove a file from source control. (full path)
  virtual void Remove(Status& status, StringParam filePath)=0;

  /// Rename / Move a file keeping it's history.
  virtual void Rename(Status& status, StringParam sourceFilePath, StringParam destFilePath)=0;

  /// Get revisions of this file
  virtual void GetRevisions(Status& status,  StringParam filePath, Array<Revision>& revisions)=0;
};

int RunSimpleCommandLine(Status& status, StringParam commandLine);


SourceControl* GetSourceControl(StringParam sourceControlType);

}
