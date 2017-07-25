///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace CommonColumns
{
  const String Name = "Name";
  const String Type = "Type";
  const String Icon = "Icon";
  const String ToolTip = "ToolTip";
}

namespace Events
{
DefineEvent(DataModified);
DefineEvent(DataDestroyed);
DefineEvent(DataRemoved);
DefineEvent(DataAttemptRemove);
DefineEvent(DataAdded);
DefineEvent(DataSelected);
DefineEvent(DataSelectionModified);
DefineEvent(DataSelectionFinal);
DefineEvent(DataActivated);
DefineEvent(DataCommandsQuery);
DefineEvent(DataCommandRun);
DefineEvent(DataReplaced);
}

ZilchDefineType(DataEvent, builder, type)
{
}

ZilchDefineType(DataReplaceEvent, builder, type)
{
}

//------------------------------------------------------------- Data Base Source
DataSource::~DataSource()
{
  DataEvent e;
  mDispatcher.Dispatch(Events::DataDestroyed, &e);
}

void DataSource::CanMove(Status& status, DataEntry* source,
                         DataEntry* destination, InsertMode::Type insertMode)
{
  status.State = StatusState::Failure;
  status.Context = InsertError::NotSupported;
}

bool DataSource::Move(DataEntry* destinationEntry, Array<DataIndex>& indicesToMove,
                      InsertMode::Type insertMode)
{
  forRange(DataIndex& index, indicesToMove.All())
  {
    DataEntry* selectedEntry = ToEntry(index);
    Move(destinationEntry, selectedEntry, insertMode);
  }

  return true;
}

//------------------------------------------------------------------ List Source
//-------------------------------------------------------------------ArrayDataSource
u32 ArrayDataSource::DataEntryToArrayIndex(DataEntry* entry)
{
  u32 index = (u32)entry;
  if(index == RootIndex)
    return (u32)-1;

  return index - 1;
}

u32 ArrayDataSource::DataIndexToArrayIndex(DataIndex dataIndex)
{
  u32 index = (u32)dataIndex.Id;
  if(index == RootIndex)
    return (u32)-1;
  return index - 1;
}

DataEntry* ArrayDataSource::GetRoot()
{
  return (DataEntry*)RootIndex;
}

DataEntry* ArrayDataSource::ToEntry(DataIndex index)
{
  return (DataEntry*)index.Id;
}

DataIndex ArrayDataSource::ToIndex(DataEntry* dataEntry)
{
  return DataIndex((u64)(dataEntry));
}

DataEntry* ArrayDataSource::Parent(DataEntry* dataEntry)
{
  //everyone but the root has the parent of the root
  u64 index = (u64)dataEntry;
  if(index == RootIndex)
    return nullptr;
  return (DataEntry*)RootIndex;
}

uint ArrayDataSource::ChildCount(DataEntry* dataEntry)
{
  //only the root has children, no one else does
  u64 index = (u64)dataEntry;
  if(index == RootIndex)
    return GetArraySize();
  return 0;
}

DataEntry* ArrayDataSource::GetChild(DataEntry* dataEntry, uint index, DataEntry* prev)
{
  //have to shift the child to be at index 1 (since a data entry of NULL is considered invalid)
  uint childIndex = index + 1;
  return ToEntry(DataIndex(childIndex));
}

bool ArrayDataSource::IsExpandable(DataEntry* dataEntry)
{
  //only the root is expandable
  u64 index = (u64)dataEntry;
  if(index == RootIndex)
    return true;
  return false;
}

void ArrayDataSource::GetData(DataEntry* dataEntry, Any& any, StringParam column)
{
  u32 index = (u32)dataEntry;
  if(index == RootIndex)
    return;

  //we had to add one to avoid returning NULL, so shift back to the actual array index
  u32 arrayIndex = index - 1;
  GetData(arrayIndex, any, column);
}

bool ArrayDataSource::SetData(DataEntry* dataEntry, const Any& any, StringParam column)
{
  u32 index = (u32)dataEntry;
  if(index == RootIndex)
    return false;

  //we had to add one to avoid returning NULL, so shift back to the actual array index
  u32 arrayIndex = index - 1;
  SetData(arrayIndex, any, column);
  return true;
}

//-------------------------------------------------------------------ListSource
ListSource::ListSource()
{

};

ListSource::~ListSource()
{
  DataEvent e;
  mDispatcher.Dispatch(Events::DataDestroyed, &e);
}

void ListSource::Modified()
{
  DataEvent e;
  mDispatcher.Dispatch(Events::DataModified, &e);
}

void ListSource::Selected(DataIndex index)
{
  DataEvent e;
  mDispatcher.Dispatch(Events::DataActivated, &e);
}

// Constructor (takes in a pointer to a null terminated array of c-strings)
EnumSource::EnumSource(const cstr* names)
{
  SetEnum(names);
}

// Set Enum (takes in a pointer to a null terminated array of c-strings)
void EnumSource::SetEnum(const cstr* names)
{
  // Store the names
  mNames = names;

  // We need to compute the size of the array
  // Start off assuming there are no elements
  mSize = 0;

  // Start off at the first name
  const cstr* start = mNames;

  // Compute the size by looping until we find the null terminator
  while (*start != nullptr)
  {
    // Increment the size and start
    ++mSize;
    ++start;
  }
}

EnumSource::EnumSource()
{
  mNames = nullptr;
  mSize = 0;
}

// Constructor (takes in a pointer to an array of c-strings, and a size)
EnumSource::EnumSource(const cstr* names, size_t size)
{
  // Store the names
  mNames = names;

  // Store the size
  mSize = size;
}

void EnumSource::SetEnum(const cstr* names, size_t size)
{
  // Store the names
  mNames = names;

  // Store the size
  mSize = size;
}

// Get the number of elements in the enum
uint EnumSource::GetCount()
{
  // Return the computed size
  return mSize;
}

// Get the name at a particular index
String EnumSource::GetStringValueAt(DataIndex index)
{
  ErrorIf(index.Id > GetCount(), "Indexing past the bounds of a static array");
  return mNames[index.Id];
}

// Add spaces to any enum with names that are upper cammel case
String AddSpacesToUpperCamelCase::Transform(StringParam input)
{
  // A string builder lets us efficiently add characters together
  StringBuilder builder;
  StringRange strRange = input;

  // Was the previous entry caps?
  bool wasCaps = true;

  // Loop through all the input
  while (!strRange.Empty())
  {
    // Is the current character upper-case?
    bool isCaps = (strRange.IsCurrentRuneUpper());

    // If the previous character was not upper case, and the
    // current character is upper case, then we need to add a space
    if (wasCaps == false && isCaps == true)
    {
      // Add the space to the string builder
      builder.Append(' ');
    }

    // Add the character that we're on to the string builder
    builder.Append(strRange.Front());

    // Roll over the caps value to the next loop
    wasCaps = isCaps;

    strRange.PopFront();
  }

  // Return the string builder's string
  return builder.ToString();
}

}