///////////////////////////////////////////////////////////////////////////////
///
/// \file Document.cpp
/// Implementation of the Document class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(DocumentRemoved);
  DefineEvent(DocumentReload);
}

ZilchDefineType(Document, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

Document::Document(u64 id)
  : Id(id)
{
  DocumentManager* docManager = DocumentManager::GetInstance();
  docManager->AddDocument(this);
  mEditCounter = 0;
  mEditor = nullptr;
}

Document::Document()
{
  Id = (u64)this;
  DocumentManager* docManager = DocumentManager::GetInstance();
  docManager->AddDocument(this);
  mEditCounter = 0;
  mEditor = nullptr;
}

Document::~Document()
{
  DocumentManager* docManager = DocumentManager::GetInstance();
  if (docManager->Documents.FindValue(Id, nullptr) == this)
    docManager->Documents.Erase(Id);
}

void Document::ReloadEditor()
{
  ObjectEvent e(this);
  DispatchEvent(Events::DocumentReload, &e);
}

//------------------------------------------------------------  DocumentManager
ZilchDefineType(DocumentManager, builder, type)
{
}

DocumentManager::DocumentManager()
{
  CurrentEditor = nullptr;
  ConnectThisTo(Z::gResources, Events::ResourceRemoved, OnResourceRemoved);
  ConnectThisTo(Z::gResources, Events::ResourceModified, OnResourceModified);
}

DocumentManager::~DocumentManager()
{

}

void DocumentManager::AddDocument(Document* document)
{
  Documents[document->Id] = document;
}

void DocumentManager::OnResourceRemoved(ResourceEvent* event)
{
  Document* document = Documents.FindValue((u64)event->EventResource->mResourceId, nullptr);
  if (document)
  {
    ObjectEvent e(this);
    document->DispatchEvent(Events::DocumentRemoved, &e);
    Documents.Erase(document->Id);
    ErrorIf(document->mEditCounter!=0, "Still edit.");
    delete document;
  }
}

void DocumentManager::OnResourceModified(ResourceEvent* event)
{
  Document* document = Documents.FindValue((u64)event->EventResource->mResourceId, nullptr);
  if (document)
    document->ReloadEditor();
}

//------------------------------------------------------------ String Document
StringDocument::StringDocument()
{

}

StringRange StringDocument::GetTextData()
{
  return mData.All();
}

String StringDocument::GetDisplayName()
{
  return mName;
}

String StringDocument::FileType()
{
  return "Text";
}

void StringDocument::Save(StringRange data)
{

}

String StringDocument::GetPath()
{
  return String();
}

DocumentResource* StringDocument::GetResource()
{
  return nullptr;
}

//------------------------------------------------------------Resource Document

ResourceDocument::ResourceDocument(DocumentResource* resource)
  : Document((u64)resource->mResourceId)
{
  mResource = resource;
}

StringRange ResourceDocument::GetTextData()
{
  return mResource->LoadTextData();
}

String ResourceDocument::GetDisplayName()
{
  return mResource->Name;
}

String ResourceDocument::FileType()
{
  return mResource->GetFormat();
}

void ResourceDocument::Save(StringRange data)
{
  mResource->SetAndSaveData(data);
}

String  ResourceDocument::GetPath()
{
  return mResource->LoadPath;
}

ICodeInspector* ResourceDocument::GetCodeInspector()
{
  return mResource->GetCodeInspector();
}

DocumentResource* ResourceDocument::GetResource()
{
  return mResource;
}

//------------------------------------------------------------File Document

FileDocument::FileDocument(StringParam name, StringParam fullPath)
{
  mPath = fullPath;
  mData = ReadFileIntoString(fullPath.c_str());
  mName = name;
}

StringRange FileDocument::GetTextData()
{
  return mData.All();
}

String FileDocument::GetDisplayName()
{
  return mName;
}

String FileDocument::FileType()
{
  return "Text";
}

void FileDocument::Save(StringRange data)
{
  mData = data;
  WriteStringRangeToFile(mPath, data);
}

String FileDocument::GetPath()
{
  return mPath;
}

DocumentResource* FileDocument::GetResource()
{
  return nullptr;
}

}//namespace Zero
