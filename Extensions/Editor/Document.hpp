///////////////////////////////////////////////////////////////////////////////
///
/// \file Document.hpp
/// Declaration of Document class. 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum3(DocumentState, New, Saved, Modified);

namespace Events
{
DeclareEvent(DocumentRemoved);
DeclareEvent(DocumentReload);

}

class DocumentEditor;
class ResourceEvent;
class ICodeInspector;

/// Document interface for managing the editing of text, script, and other files.
class Document : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Document(u64 id);
  Document();
  virtual ~Document();

  //Get the readable simple name of the document.
  virtual String GetDisplayName() = 0;

  //Save the new data.
  virtual void Save(StringRange data) = 0;

  //Get the entire document as a string
  virtual StringRange GetTextData() = 0;

  //File type of syntax to use for highlighting.
  virtual String FileType() = 0;

  //Get the resource for this document.
  virtual DocumentResource* GetResource() { return nullptr; }
  virtual String GetPath() { return String(); }

  //Get the document state.
  DocumentState::Type GetState(){return mState;}

  /// This will signal a refresh on any open text editors.
  void ReloadEditor();

  /// Gets a code inspector which is used for code completion and other code editing features
  virtual ICodeInspector* GetCodeInspector() { return nullptr; }

  int mEditCounter;
  DocumentEditor* mEditor;

protected:
  String mName;

private:
  u64 Id;
  DocumentState::Type mState;
  friend class DocumentManager;
};

class DocumentManager : public ExplicitSingleton<DocumentManager, EventObject>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DocumentManager();
  ~DocumentManager();

  void AddDocument(Document* document);
  HashMap<u64, Document*> Documents;
  DocumentEditor* CurrentEditor;
  PodArray<DocumentEditor*> Instances;

  void OnResourceRemoved(ResourceEvent* event);
  void OnResourceModified(ResourceEvent* event);
};

/// A Document that is just a string in memory.
class StringDocument : public Document
{
public:
  StringDocument();

  //Document Interface
  StringRange GetTextData() override;
  String GetDisplayName() override;
  String FileType() override;
  void Save(StringRange data) override;
  String GetPath() override;
  DocumentResource* GetResource() override;

  String mName;
  String mData;
};

/// Document that is connected to an engine resource.
class ResourceDocument : public Document
{
public:
  ResourceDocument(DocumentResource* resource);

  //Document Interface
  StringRange GetTextData() override;
  String GetDisplayName() override;
  String FileType() override;
  void Save(StringRange data) override;
  String GetPath() override;
  ICodeInspector* GetCodeInspector() override;

  DocumentResource* GetResource() override;
  DocumentResource* mResource;
};

/// Document that is just a simple file.
class FileDocument : public Document
{
public:
  FileDocument(StringParam name, StringParam fullPath);

  //Document Interface
  StringRange GetTextData() override;
  String GetDisplayName() override;
  String FileType() override;
  void Save(StringRange data) override;
  String GetPath() override;
  DocumentResource* GetResource() override;

  String mName;
  String mData;
  String mPath;
};


}//namespace Zero
