///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Data Source that filters another DataSource
class DataSourceFilter : public DataSource
{
public:
  Array<DataIndex> mFilteredList;
  DataSource* mSource;
  String mFilterString;

  DataSourceFilter()
  {
    mSource = NULL;
  }

  // Helper bind for member functions bases filters
  template<typename type, bool (type::*Func)(DataEntry* entry)>
  class BindMethodPtr
  {
  public:
    BindMethodPtr(type* instance)
      :mInstance(instance)
    {
    }

    type* mInstance;

    bool operator()(DataEntry* entry)
    {
      return (mInstance->*Func)(entry);
    }
  };

  // Filter the nodes with the give filter functor
  template<typename filtertype>
  void FilterNodes(filtertype filter, DataEntry* node)
  {
    DataEntry* prev = NULL;

    uint childCount = mSource->ChildCount(node);

    for(uint i=0;i<childCount;++i)
    {
      DataEntry* child = mSource->GetChild(node, i, prev);

      if(filter(child))
        mFilteredList.PushBack(mSource->ToIndex(child));

      uint numbChildren = mSource->ChildCount(child);
      if(numbChildren > 0)
        FilterNodes(filter, child);

      prev = child;
    }
  }

  // Basic filter by name
  bool FilterByName(DataEntry* entry)
  {
    Any varName;
    mSource->GetData(entry, varName, CommonColumns::Name);

    String name = varName.Get<String>();
    int priority = PartialMatch(mFilterString.All(), name.All(), CaseInsensitiveCompare);
    return priority != cNoMatch;
  }

  // Filter with the given string
  virtual void Filter(StringParam filterString)
  {
    mFilteredList.Clear();
    mFilterString = filterString;

    // Default is to filter by name
    DataEntry* root = mSource->GetRoot();
    BindMethodPtr<DataSourceFilter, &DataSourceFilter::FilterByName> nameFilter(this);
    FilterNodes(nameFilter, root);
  }
  
  virtual void SetSource(DataSource* dataSource)
  {
    mSource = dataSource;
    mFilteredList.Clear();
  }

  // Rest is just an adapter to original interface

  DataEntry* GetRoot() override
  {
    return this;
  }

  //Safe Indexing
  DataEntry* ToEntry(DataIndex index) override
  {
    if(index == cRootIndex)
      return this;

    return mSource->ToEntry(index);
  }

  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    if(dataEntry == this)
      return cRootIndex;

    return mSource->ToIndex(dataEntry);
  }

  DataEntry* Parent(DataEntry* dataEntry) override
  {
    return NULL;
  }

  uint ChildCount(DataEntry* dataEntry) override
  {
    if(dataEntry == this)
      return mFilteredList.Size();
    else
      return 0;
  }

  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    ErrorIf(dataEntry != this, "FilteredDataSource is flat");
    return mSource->ToEntry(mFilteredList[index]);
  }

  //Tree expanding
  bool IsExpandable(DataEntry* dataEntry) override {return dataEntry == this;}

  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    if(dataEntry == this)
    {
      if(column == CommonColumns::Icon)
        variant = "ItemIcon";

      if(column == CommonColumns::Type)
        variant = "Object";
      return;
    }

    mSource->GetData(dataEntry, variant, column);
  }

  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    if(dataEntry == this)
      return false;

    mSource->SetData(dataEntry, variant, column);
    return true;
  }

};

}
