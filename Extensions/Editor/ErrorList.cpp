///////////////////////////////////////////////////////////////////////////////
///
/// \file ObjectView.cpp
/// 
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Column
{
  const String Icon = "Icon";
  const String Description = "Description";
  const String File = "File";
  const String Line = "Line";
  const String Library = "Library";
}

struct ScriptError
{
  String Description;
  String File;
  uint Line;
  String Library;
};

//------------------------------------------------------------------ Data Source
class ErrorListSource : public DataSource
{
public:
  Array<ScriptError*> mErrors;

  ErrorListSource()
  {
    // Root
    mErrors.PushBack(new ScriptError());

    for(uint i = 0; i < 10; ++i)
    {
      ScriptError* error = new ScriptError();
      error->Description = "expected an indented block";
      error->File = "Test.py";
      error->Line = i * 3 - i;
      mErrors.PushBack(error);
    }
  }

  //DataSource Interface
  
  //Data base Indexing
  DataEntry* GetRoot() override
  {
    return mErrors[0];
  }

  //Safe Indexing
  DataEntry* ToEntry(DataIndex index) override
  {
    return mErrors[(uint)index.Id];
  }

  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    ScriptError* error = (ScriptError*)dataEntry;
    return mErrors.FindIndex(error);
  }

  DataEntry* Parent(DataEntry* dataEntry) override
  {
    ScriptError* root = mErrors[0];
    if(dataEntry == root)
      return NULL;
    return root;
  }

  uint ChildCount(DataEntry* dataEntry) override
  {
    ScriptError* root = mErrors[0];
    if(dataEntry == root)
      return mErrors.Size() - 1;
    return 0;
  }

  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    ScriptError* root = mErrors[0];
    if(dataEntry == root)
      return mErrors[index + 1];
    return NULL;
  }

  //Tree expanding
  bool IsExpandable(DataEntry* dataEntry) override
  {
    ScriptError* root = mErrors[0];
    if(dataEntry == root)
      return true;
    return false;
  }

  //Data Base Cell Modification and Inspection
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    //error C2061:
    ScriptError* error = (ScriptError*)dataEntry;

    if(column == Column::Icon)
    {
      variant = "RedX";
    }
    if(column == Column::Description)
    {
      variant =  error->Description;
    }
    if(column == Column::File)
    {
      variant = error->File;
    }
    if(column == Column::Line)
    {
      variant = String::Format("%i", error->Line);
    }
    if(column == Column::Library)
    {
      variant = "FrozenTestBed";
    }
  }

  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    // Cannot modify anything
    return false;
  }
};

//------------------------------------------------------------------ Object View
ErrorList::ErrorList(Composite* parent)
  :Composite(parent)
{
  this->SetLayout(CreateStackLayout());
  mTree = new TreeView(this);
  mSource = NULL;

  TreeFormatting formatting;
  BuildFormat(formatting);
  mTree->SetFormat(formatting);
  mSource = new ErrorListSource();
  mTree->SetDataSource(mSource);
}

ErrorList::~ErrorList()
{
  SafeDelete(mSource);
}

void ErrorList::ClearErrors()
{
  forRange(ScriptError* error, mSource->mErrors.All())
  {
    delete error;
  }

  mSource->mErrors.Clear();
}

void ErrorList::OnScriptError(Event* event)
{
  ScriptError* error = new ScriptError();
  // This was originally python specific, this should be remade to be more generic
  //error->Description = event->Message;
  //error->File = FilePath::GetFileName(event->Location.FileName);
  //error->Line = event->Location.LineNumber;
  mSource->mErrors.PushBack(error);
  mTree->Refresh();
}

void ErrorList::UpdateTransform()
{
  Composite::UpdateTransform();
}

void ErrorList::BuildFormat(TreeFormatting& formatting)
{
  formatting.Flags.SetFlag(FormatFlags::ShowHeaders | FormatFlags::ShowSeparators);

  // Icon
  ColumnFormat* format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = Column::Icon;
  format->ColumnType = ColumnType::Sizeable;
  format->FixedSize = Pixels(20, 20);
  format->Editable = false;
  format->CustomEditor = cDefaultIconEditor;

  // Description
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = Column::Description;
  format->HeaderName = format->Name;
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 1;
  format->Editable = false;

  // File
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = Column::File;
  format->HeaderName = format->Name;
  format->ColumnType = ColumnType::Sizeable;
  format->FixedSize = Pixels(90, 20);
  format->Editable = false;

  // Line
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = Column::Line;
  format->HeaderName = format->Name;
  format->ColumnType = ColumnType::Sizeable;
  format->FixedSize = Pixels(60, 20);
  format->Editable = false;

  // Library
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = Column::Library;
  format->HeaderName = format->Name;
  format->ColumnType = ColumnType::Sizeable;
  format->FixedSize = Pixels(110, 20);
  format->Editable = false;
}

void ErrorList::OnDataActivated(DataEvent* event)
{
  // Open the file and go to that error
}

}//namespace Zero
