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

struct WatchEntry
{
  String Name;
  String Value;
  String Type;
  WatchEntry* Parent;
  Array<WatchEntry*> Children;
};

//------------------------------------------------------------------ Data Source
#define AddEntry(varName, name, value, type, parent) \
  WatchEntry* varName = new WatchEntry(); \
  varName->Name = name; \
  varName->Value = value; \
  varName->Type = type; \
  varName->Parent = parent; \
  if(parent) \
    parent->Children.PushBack(varName); \
  mEntries.PushBack(varName);

class WatchViewSource : public DataSource
{
public:
  Array<WatchEntry*> mEntries;

  WatchViewSource()
  {
    WatchEntry* nullPtr = NULL;
    // Root
    AddEntry(rootEntry, "root", "", "", nullPtr);
    AddEntry(thisEntry, "this", "0x0cb4ca38 {Position={...} }", "Zero::PlayerController* const", rootEntry);
    AddEntry(pos, "Position", "{x=5.0999999, y=16.000000, z=7.5000000 ...}", "Math::Vector3", thisEntry);
    AddEntry(posX, "x", "5.0999999", "float", pos);
    AddEntry(posY, "y", "16.000000", "float", pos);
    AddEntry(posZ, "z", "7.5000000", "float", pos);
    AddEntry(thisPosEntry, "this->Position", "{x=5.0999999, y=16.000000, z=7.5000000 ...}", "Math::Vector3", rootEntry);
    AddEntry(posX2, "x", "5.0999999", "float", thisPosEntry);
    AddEntry(posY2, "y", "16.000000", "float", thisPosEntry);
    AddEntry(posZ2, "z", "7.5000000", "float", thisPosEntry);
  }

  //DataSource Interface
  
  //Data base Indexing
  DataEntry* GetRoot() override
  {
    return mEntries[0];
  }

  //Safe Indexing
  DataEntry* ToEntry(DataIndex index) override
  {
    return mEntries[(uint)index.Id];
  }

  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    WatchEntry* entry = (WatchEntry*)dataEntry;
    return mEntries.FindIndex(entry);
  }

  DataEntry* Parent(DataEntry* dataEntry) override
  {
    WatchEntry* entry = (WatchEntry*)(dataEntry);
    return entry->Parent;
  }

  uint ChildCount(DataEntry* dataEntry) override
  {
    WatchEntry* entry = (WatchEntry*)(dataEntry);
    return entry->Children.Size();
  }

  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    WatchEntry* entry = (WatchEntry*)(dataEntry);
    return entry->Children[index];
  }

  //Tree expanding
  bool IsExpandable(DataEntry* dataEntry) override
  {
    WatchEntry* entry = (WatchEntry*)(dataEntry);
    return !entry->Children.Empty();
  }

  //Data Base Cell Modification and Inspection
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    WatchEntry* entry = (WatchEntry*)dataEntry;
    
    if(column == "Icon")
    {
      if(entry->Children.Empty())
        variant = "ItemIcon";
      else
        variant = "Model";
    }
    if(column == "Name")
    {
      variant =  entry->Name;
    }
    if(column == "Value")
    {
      variant = entry->Value;
    }
    if(column == "Type")
    {
      variant = entry->Type;
    }
  }

  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    WatchEntry* entry = (WatchEntry*)dataEntry;
    bool result = false;
    if(column == "Name")
    {
      entry->Name = variant.Get<String>();
      result = true;
    }
    if(column == "Value")
    {
      entry->Value = variant.Get<String>();
      result = true;
    }
    if(column == "Type")
    {
      entry->Type = variant.Get<String>();
      result = true;
    }
    return result;
  }
};

//------------------------------------------------------------------ Object View
WatchView::WatchView(Composite* parent)
  :Composite(parent)
{
  mTree = new TreeView(this);
  mSource = NULL;

  TreeFormatting formatting;
  BuildFormat(formatting);
  mTree->SetFormat(formatting);
  mSource = new WatchViewSource();
  mTree->SetDataSource(mSource);

  this->SetLayout(CreateStackLayout());
}

WatchView::~WatchView()
{
  SafeDelete(mSource);
}

void WatchView::BuildFormat(TreeFormatting& formatting)
{
  formatting.Flags.SetFlag(FormatFlags::ShowHeaders | 
                           FormatFlags::ShowSeparators | 
                           FormatFlags::ColumnsResizable);

  // Icon
  ColumnFormat* format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = "Icon";
  format->ColumnType = ColumnType::Fixed;
  format->FixedSize = Pixels(20, 20);
  format->Editable = false;
  format->CustomEditor = cDefaultIconEditor;

  // Name
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = "Name";
  format->HeaderName = format->Name;
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 1;
  format->Editable = true;

  // Value
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = "Value";
  format->HeaderName = format->Name;
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 2;
  format->Editable = true;

  // Type
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = "Type";
  format->HeaderName = format->Name;
  format->ColumnType = ColumnType::Sizeable;
  format->FixedSize = Pixels(160, 20);
  format->Editable = false;
}

}//namespace Zero
