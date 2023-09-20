// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

typedef void* DataItem;
typedef void* ContextPtr;
class MetaDropEvent;

// Safe handle for elements in a data base.
struct DataIndex
{
  DataIndex() : Id(0)
  {
  }
  DataIndex(u64 id) : Id(id)
  {
  }
  u64 Id;
  bool IsValid()
  {
    return Id != 0;
  }
  bool operator==(const DataIndex& other)
  {
    return Id == other.Id;
  }
};

// Data events
namespace Events
{
// Data source has been invalided and must be refreshed.
DeclareEvent(DataModified);
// Data source destroyed
DeclareEvent(DataDestroyed);
// Data Index Removed
DeclareEvent(DataRemoved);
// Attempt Data Remove
DeclareEvent(DataAttemptRemove);
// Data Added After Index
DeclareEvent(DataAdded);
// Data has been selected in view.
DeclareEvent(DataSelected);
// Data has been selected / deselected in view.
DeclareEvent(DataSelectionModified);
DeclareEvent(DataSelectionFinal);
// Data has been 'Activated' in view by being double clicked.
DeclareEvent(DataActivated);
// Query for what commands can be run on index.
DeclareEvent(DataCommandsQuery);
// Run a command on a index
DeclareEvent(DataCommandRun);
// Data has been replaced (i.e. when an Archetype is rebuilt)
DeclareEvent(DataReplaced);
} // namespace Events

class DataEvent : public Event
{
public:
  RaverieDeclareType(DataEvent, TypeCopyMode::ReferenceType);
  DataIndex Index;
  String Command;
};

class DataReplaceEvent : public Event
{
public:
  RaverieDeclareType(DataReplaceEvent, TypeCopyMode::ReferenceType);
  DataIndex mOldIndex;
  DataIndex mNewIndex;
};

typedef void DataEntry;

// Common Columns
namespace CommonColumns
{
// Name of Entry
extern const String Name;
// Type of Entry
extern const String Type;
// Icon to use for entry
extern const String Icon;
// Used for display tooltips on data
extern const String ToolTip;
} // namespace CommonColumns

DeclareEnum3(InsertMode, On, After, Before);

// Used as an extended error code for the CanMove function on DataSource.
// 'NotSupported' means it will attempt other Insert modes depending on where
// the mouse is.
// For example, if 'NotSupported' is set from an 'InsertMode::On' query,
// it will then attempt to Insert 'On' or 'After'.
// If 'Invalid' is set, it will simply display the error tooltip.
DeclareEnum3(InsertError, None, Invalid, NotSupported);

// Value that can be used for the root index
const DataIndex cRootIndex = DataIndex(u64(-1));

// Data Base Source is used to access tree data sources.
class DataSource : public EventObject
{
public:
  RaverieDeclareType(DataSource, TypeCopyMode::ReferenceType);
  DataSource()
  {
  }
  virtual ~DataSource();

  /// Data base Indexing
  virtual DataEntry* FindEntryByName(StringParam name)
  {
    return nullptr;
  }
  virtual DataEntry* GetRoot() = 0;

  /// Safe Indexing
  virtual DataEntry* ToEntry(DataIndex index) = 0;
  virtual DataIndex ToIndex(DataEntry* dataEntry) = 0;
  virtual Handle ToHandle(DataEntry* dataEntry)
  {
    return Handle();
  }

  /// Tree Walking
  virtual DataEntry* Parent(DataEntry* dataEntry) = 0;
  virtual uint ChildCount(DataEntry* dataEntry) = 0;
  virtual DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) = 0;

  /// Query to see if the data source has need for expandable rows.
  ///   Ex: The TreeView can check it's associated data source, and if this
  ///       returns false - each TreeRow's UI will not reserve an area in their
  ///       row to display the expand/collapse icon.
  virtual bool IsExpandable()
  {
    return true;
  }
  /// Tree expanding
  virtual bool IsExpandable(DataEntry* dataEntry) = 0;
  virtual void Expand(DataEntry* dataEntry)
  {
  }
  virtual void Collapse(DataEntry* dataEntry)
  {
  }

  /// Data Base Cell Modification and Inspection
  virtual void GetData(DataEntry* dataEntry, Any& variant, StringParam column) = 0;
  virtual bool SetData(DataEntry* dataEntry, const Any& variant, StringParam column) = 0;

  /// Sorting
  /// Short children of entry by column. If the dataEntry is null, sort
  /// everything
  virtual void Sort(DataEntry* dataEntry, StringParam column, bool sortFlip){};

  /// Tree Modification
  virtual void CanMove(Status& status, DataEntry* source, DataEntry* destination, InsertMode::Type insertMode);
  virtual void BeginBatchMove()
  {
  }
  virtual bool Move(DataEntry* destinationEntry, DataEntry* movingEntry, InsertMode::Type insertMode)
  {
    return false;
  }
  virtual bool Move(DataEntry* destinationEntry, Array<DataIndex>& indicesToMove, InsertMode::Type insertMode);
  virtual void EndBatchMove()
  {
  }
  virtual bool Remove(DataEntry* dataEntry)
  {
    return false;
  }

  /// Whether or not the entry is considered valid. Used to disable or
  /// invalidate elements of the tree.
  virtual bool IsValid(DataEntry* entry)
  {
    return true;
  }

  /// Allows the data source to handle custom drops on rows.
  virtual void OnMetaDrop(MetaDropEvent* e, DataEntry* mouseOver, InsertMode::Enum mode)
  {
  }
};

// A data source to wrap an array.
class ArrayDataSource : public DataSource
{
public:
  static const u64 RootIndex = (u64)-1;

  /// Return the size of the contained array
  virtual u64 GetArraySize() = 0;
  /// Fills out the variant with the data of the given column at the given array
  /// index
  virtual void GetData(u64 index, Any& any, StringParam column) = 0;
  /// Sets the data of the given column at the given array index with the passed
  /// in variant.
  virtual void SetData(u64 index, const Any& any, StringParam column) = 0;

  // Helpers to get an array index from an entry or index
  u64 DataEntryToArrayIndex(DataEntry* entry);
  u64 DataIndexToArrayIndex(DataIndex index);

  // DataSource Interface
  DataEntry* GetRoot() override;
  DataEntry* ToEntry(DataIndex index) override;
  DataIndex ToIndex(DataEntry* dataEntry) override;
  DataEntry* Parent(DataEntry* dataEntry) override;
  uint ChildCount(DataEntry* dataEntry) override;
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override;
  bool IsExpandable(DataEntry* dataEntry) override;
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override;
  bool SetData(DataEntry* dataEntry, const Any& variant, StringParam column) override;
};

/// Data Index Selection Object. Interface for managing a DataSource's
/// selection state.
class DataSelection : public EventObject
{
public:
  DataSource* mSource;
  DataSelection() : mSupportsMultiSelect(true)
  {
  }
  virtual ~DataSelection(){};

  /// Get all selected objects
  virtual void GetSelected(Array<DataIndex>& selected) = 0;
  /// How many objects are selected.
  virtual uint Size() = 0;
  /// Is this data index selected?
  virtual bool IsSelected(DataIndex index) = 0;
  /// Select this index.
  virtual void Select(DataIndex index, bool sendsEvents = true) = 0;
  /// Deselect this index.
  virtual void Deselect(DataIndex index) = 0;
  /// Clear the selection
  virtual void SelectNone(bool sendsEvents = true) = 0;

  /// Selection modified
  virtual void SelectionModified()
  {
    ObjectEvent eventToSend(this);
    GetDispatcher()->Dispatch(Events::DataSelectionModified, &eventToSend);
  }

  /// Selection Final
  virtual void SelectFinal()
  {
    ObjectEvent eventToSend(this);
    GetDispatcher()->Dispatch(Events::DataSelectionFinal, &eventToSend);
  }

  bool mSupportsMultiSelect;
};

/// Simple DataSelection that uses a hash map.
class HashDataSelection : public DataSelection
{
public:
  HashSet<u64> mSelection;

  void GetSelected(Array<DataIndex>& selected) override
  {
    forRange (u64 u, mSelection.All())
      selected.PushBack(u);
  }

  bool IsSelected(DataIndex index) override
  {
    return mSelection.Count(index.Id) > 0;
  }

  void Select(DataIndex index, bool sendsEvents = true) override
  {
    mSelection.Insert(index.Id);
  }

  void Deselect(DataIndex index) override
  {
    mSelection.Erase(index.Id);
  }

  void SelectNone(bool sendsEvents = true) override
  {
    mSelection.Clear();
  }

  uint Size() override
  {
    return mSelection.Size();
  }
};

class SingleDataSelection : public DataSelection
{
public:
  DataIndex mSelected;

  SingleDataSelection()
  {
    mSupportsMultiSelect = false;
  }

  void GetSelected(Array<DataIndex>& selected) override
  {
    if (mSelected.IsValid())
      selected.PushBack(mSelected);
  }

  bool IsSelected(DataIndex index) override
  {
    return (mSelected == index);
  }

  void Select(DataIndex index, bool sendsEvents = true) override
  {
    mSelected = index;
  }

  void Deselect(DataIndex index) override
  {
    mSelected = DataIndex();
  }

  void SelectNone(bool sendsEvents = true) override
  {
    mSelected = DataIndex();
  }

  uint Size() override
  {
    if (mSelected.IsValid())
      return 1;
    return 0;
  }
};

/// ListSource class for list boxes
class ListSource : public EventObject
{
public:
  ListSource();
  ~ListSource();

  void Modified();

  virtual uint GetCount() = 0;
  virtual void Selected(DataIndex index);
  virtual String GetStringValueAt(DataIndex index) = 0;
  virtual String GetDescriptionAt(DataIndex index)
  {
    return "";
  };
};

struct ContainerToString
{
  template <typename type>
  String Convert(const type& instance)
  {
    return ToString(instance);
  }
};

template <typename type, typename StringConverter = ContainerToString>
class ContainerSource : public ListSource
{
};

template <typename type, typename StringConverter>
class ContainerSource<Array<type>, StringConverter> : public ListSource
{
public:
  typedef Array<type> containerType;
  StringConverter mConverter;
  ContainerSource(Array<type>* data)
  {
    mData = data;
  }

  containerType* mData;

  void SetSource(containerType* data)
  {
    mData = data;
  }
  virtual bool IsCollection()
  {
    return true;
  }
  virtual uint GetCount()
  {
    return mData->Size();
  }

  uint Size()
  {
    return mData->Size();
  }
  type& operator[](uint index)
  {
    return (*mData)[index];
  }
  bool Empty()
  {
    return mData->Empty();
  }
  void Clear()
  {
    mData->Clear();
  }

  String GetStringValueAt(DataIndex index) override
  {
    return mConverter.Convert((*mData)[(uint)index.Id]);
  }
};

struct CStrToString
{
  String Convert(cstr input)
  {
    return ToString(input);
  }
};

struct AddSpacesToUpperCamelCase
{
  String Convert(cstr input);
};

template <typename StringConverter>
class CStrSource : public ListSource
{
public:
  CStrSource()
  {
    mData = nullptr;
  }

  // Takes in a pointer to a null terminated array of c-strings.
  CStrSource(const cstr data[])
  {
    SetSource(data);
  }

  // Takes in a pointer to a null terminated array of c-strings.
  void SetSource(const cstr data[])
  {
    // Store the names
    mData = data;

    // Need to compute the size of the array;
    // start off assuming there are no elements.
    mSize = 0;

    // Start off at the first name
    const cstr* start = mData;

    // Compute the size by looping until we find the null terminator
    while (*start != nullptr)
    {
      // Increment the size and start
      ++mSize;
      ++start;
    }
  }

  String GetStringValueAt(DataIndex index) override
  {
    return mConverter.Convert(mData[(uint)index.Id]);
  }

  inline String operator[](uint index)
  {
    return GetStringValueAt(index);
  }

  uint Size()
  {
    return mSize;
  }
  uint GetCount() override
  {
    return mSize;
  }

  bool Empty()
  {
    return (mSize == 0);
  }

public:
  size_t mSize;
  const cstr* mData;
  StringConverter mConverter;
};

typedef CStrSource<CStrToString> CStrSourceStd;
typedef CStrSource<AddSpacesToUpperCamelCase> CStrSourceSpaced;

class StringSource : public ContainerSource<Array<String>>
{
public:
  Array<String> Strings;

  uint GetIndexOfString(StringParam string)
  {
    for (uint i = 0; i < Strings.Size(); ++i)
    {
      if (Strings[i] == string)
        return i;
    }
    return 0;
  }

  StringSource() : ContainerSource<Array<String>>(&Strings)
  {
  }
};

/// A source provider for enums.
class EnumSource : public ListSource
{
public:
  EnumSource();

  /// Build the enum info from the meta type.
  void BuildFromMeta(BoundType* enumType);

  /// Retrieve enum descriptions, if available.
  void FillOutDescriptions();

  /// Get the number of elements in the enum.
  uint GetCount() override;

  /// Get the enum name at a particular index.
  String GetStringValueAt(DataIndex index) override;
  /// Get the description (documentation) for the corresponding enum at the same
  /// index.
  String GetDescriptionAt(DataIndex index) override;

private:
  // Store the names, descriptions, and the size of the arrays
  BoundType* mEnumType;
  Array<String> mNames;
  Array<String> mDescriptions;
};

} // namespace Raverie
