///////////////////////////////////////////////////////////////////////////////
///
/// Authors:
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

template<typename type>
size_t LinearErase(Array<type>& array, type& value, size_t startIndex = 0)
{
  ErrorIf(array.Empty(), "Erase on empty array.");
  if(array.Empty())
    return size_t(-1);

  uint foundIndex=startIndex;
  for(;foundIndex < array.Size(); ++foundIndex)
  {
    if(array[foundIndex] == value)
      break;
  }
  for(uint move = foundIndex; move < array.Size() - 1; ++move)
  {
    array[move] = array[move + 1];
  }

  if(foundIndex<array.Size())
    array.Resize(array.Size() - 1);

  return foundIndex;
}

struct DataSorter
{
  Array<String>& values;
  DataSorter(Array<String>& v)
    : values(v)
  {
  }

  bool operator()(int indexA, int indexB)
  {
    return (values[indexA].ToLower()) < (values[indexB].ToLower());
  }
};

DeclareBitField3(NodeFlags, Item, Folder, Erasable);

const String cFolderIcon = "FolderIcon";

class TreeNode
{
public:
  u64 Id;
  uint Index;
  BitField<NodeFlags::Enum> Flags;
  bool IsFolder(){return Flags.IsSet(NodeFlags::Folder);}
  String Name;
};

struct MetaObjectPolicy
{
  typedef Handle StoredType;
  typedef u64 MapType;
  enum{Mappable=true};

  template<typename Tree, typename EntryType>
  static void Expand(Tree* tree, EntryType* parent)
  {
  }

  static bool IsExpandable(StoredType& object)
  {
    return false;
  }

  static MapType Map(StoredType& object)
  {
    return object.HandleU64;
  }

  static StoredType UnMap(MapType object)
  {
    return StoredType();
  }
};

template<typename type>
struct ValuePolicy
{
  typedef type StoredType;
  typedef type MapType;
  enum{Mappable=false};

  static StoredType InvalidValue()
  {
    return StoredType();
  }


  static MapType Map(StoredType& object)
  {
    return object;
  }

  static StoredType UnMap(MapType object)
  {
    return object;
  }

  template<typename Tree, typename EntryType>
  static void Expand(Tree* tree, EntryType* parent)
  {

  }

  static bool IsExpandable(StoredType& object)
  {
    return false;
  }
};


//------------------------------------------------------------  AutoDataBaseSource

///This class provides a tree container that implements the DataBaseSource interface.
template<typename Policy>
class PolicyDataSource : public DataSource
{
public:
  typedef typename Policy::StoredType ValueType;
  typedef typename Policy::MapType MapType;
  u64 mIdCount;

  class TreeNodeType : public TreeNode
  {
  public:
    TreeNodeType* Parent;
    ValueType Object;

    typename Array<TreeNodeType*>::range GetChildren()
    {
      return Children.All();
    }

    void AddChild(TreeNodeType* treeNode, uint where=-1)
    {
      treeNode->Parent = this;
      if(where > Children.Size())
      {
        treeNode->Index = Children.Size();
        Children.PushBack(treeNode);
      }
      else
      {
        Children.InsertAt(where, treeNode);
        //Fix all indices
        for(uint index = where;index<Children.Size();++index)
          Children[index]->Index = index;
      }
    }

    void RemoveChild(TreeNodeType* treeNode)
    {
      ReturnIf(treeNode->Parent!=this,,"Element is not child of this node.");
      ReturnIf(treeNode != Children[treeNode->Index],,"Element is not at index.");

      uint index = treeNode->Index;

      treeNode->Parent = NULL;
      treeNode->Index = 0;

      //Move all elements back
      for(;index<Children.Size()-1;++index)
      {
        Children[index] = Children[index+1];
        Children[index]->Index = index;
      }
      Children.Resize(Children.Size()-1);
    }

    TreeNodeType* FindChild(const ValueType& value)
    {
      forRange(TreeNodeType* child, GetChildren())
      {
        if (child->Object == value)
          return child;
      }

      return nullptr;
    }

    uint ChildCount()
    {
      return Children.Size();
    }

    Array<TreeNodeType*> Children;
  };

  PolicyDataSource(MapType map)
  {
    mIdCount = 0;
    mRoot = NewNode(NULL, map);
  }

  PolicyDataSource()
  {
    mIdCount = 0;
    mRoot = CreateNode(NULL, NodeFlags::Folder, Policy::InvalidValue());
    mRoot->Flags.SetFlag(NodeFlags::Folder);
    mRoot->Name = "Root";
  }

  ~PolicyDataSource()
  {
    DeleteObjectsInContainer(IdToEntry);
  }

  void AddRoot(const ValueType& a)
  {
    NewNode(mRoot, a);
  }

  void Save(TreeNodeType* node, Serializer& serializer)
  {

    if(node->Flags.IsSet(NodeFlags::Folder))
    {
      serializer.StartPolymorphic("Folder", 0, 0);
      serializer.SerializeField("Name", node->Name); 
      for(uint i=0;i<node->Children.Size();++i)
      {
        Save(node->Children[i], serializer);
      }
      serializer.EndPolymorphic();
    }
    else
    {
      serializer.StartPolymorphic("Node", 0, 0);
      MapType mapType = Policy::Map(node->Object);
      serializer.SerializeField("Value", mapType);
      serializer.EndPolymorphic();
    }

  }

  void Save(StringParam filename)
  {
    Status status;
    Serializer* stream = GetSaverStreamFile(status, filename);
    if(stream)
    {
      Save(mRoot, *stream);
      delete stream;
    }
  }

  void Load(TreeNodeType* parent, Serializer& serializer)
  {
    PolymorphicNode snode;
    while(serializer.GetPolymorphic(snode))
    {
      if(snode.TypeName == "Folder")
      {
        TreeNodeType* node = CreateNode(parent, NodeFlags::Folder, 0);
        serializer.SerializeField("Name", node->Name); 
        Load(node, serializer);
        serializer.EndPolymorphic();
      }
      else
      {
        MapType mapType;
        serializer.SerializeField("Value", mapType);
        ValueType valueType = Policy::UnMap(mapType);
        if(valueType!=Policy::InvalidValue())
          CreateNode(parent, NodeFlags::Item, valueType);
        serializer.EndPolymorphic();
      }
    }
  }


  bool Load(Status& status, StringParam fileName)
  {
    UniquePointer<Serializer> stream(GetLoaderStreamFile(status, fileName));
    if(stream)
    {
      Load(NULL, *stream);
      return true;
    }
    else
      return false;
  }

  DataEntry* Parent(DataEntry* dataEntry)
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    return node->Parent;
  }

  TreeNodeType* mRoot;

  TreeNodeType* GetRootNode()
  {
    return mRoot;
  }

  //Use id map(same as objects)
  HashMap<u64, TreeNodeType*> IdToEntry;
  HashMap<MapType, TreeNodeType*> ObjectToEntry;

  void ModifiedNode(TreeNodeType* node)
  {
    EventOnNode(node, Events::DataModified);
  }

  void EraseNode(TreeNodeType* node, bool external)
  {
    TreeNodeType* parent = node->Parent;
 
    //Erase out of Map
    if(node->Object != Policy::InvalidValue())
    {
      if(Policy::Mappable)
        ObjectToEntry.Erase(Policy::Map(node->Object));

      //Erase the object
      if(!external)
        Policy::Erase(node, node->Object);
    }

    //Erase the node out of map
    IdToEntry.Erase(node->Id);

    //Erase node out of child array
    parent->RemoveChild(node);

    //Signal this node as removed
    EventOnNode(node, Events::DataRemoved);

    //Safe to delete
    delete node;
  }

  void EventOnNode(TreeNodeType* node, StringParam event)
  {
    DataEvent e;
    e.Index = node->Id;
    this->DispatchEvent(event, &e);
  }

  TreeNodeType* CreateNode(TreeNodeType* parent, uint flags, ValueType object)
  {
    //Construct a new node

    //Every node gets unique id
    u64 id = ++mIdCount;

    TreeNodeType* node = new TreeNodeType();
    node->Id = id;
    node->Object = object;
    node->Parent = parent;
    node->Flags.U32Field = flags;
    node->Index = 0;
    node->Name = object;

    //Map it for handles
    IdToEntry[id] = node;

    //If the value is not the default map it
    if(Policy::Mappable && object != Policy::InvalidValue())
      ObjectToEntry[ Policy::Map(object) ] = node;

    //Add to the parent's list
    if(parent)
    {
      parent->AddChild(node);
    }
    else
      mRoot = node;

    //Send data added.
    if(parent)
      EventOnNode(parent, Events::DataAdded);

    return node;
  }

  TreeNodeType* AddItem(TreeNodeType* parent, ValueType object)
  {
    return CreateNode(parent, NodeFlags::Item, object);
  }

  TreeNodeType* ToNode(MapType& mapIndex)
  {
    return ObjectToEntry[mapIndex];
  }

  TreeNodeType* ToNode(DataIndex& index)
  {
    return IdToEntry[index.Id];
  }

  ValueType& GetEntry(DataIndex& index)
  {
    return IdToEntry[index.Id]->Object;
  }

  //------------------------------------------------------------

  //Data Source Interface
  DataEntry* GetRoot() override
  {
    return mRoot;
  }

  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    return node->Id;
  }

  DataEntry* ToEntry(DataIndex index) override
  {
    return IdToEntry[index.Id];
  }

  Handle ToInstance(DataEntry* dataEntry)
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    if(node->Object)
      return node->Object;
    else
      return Handle();
  }
  //------------------------------------------------------------ 
  
  //Data Base Interface

  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    if(node->IsFolder())
    {
      if(column == CommonColumns::Name)
        variant = node->Name;
      if(column == CommonColumns::Icon)
        variant = cFolderIcon;
      return;
    }
    else
    {
      return Policy::GetData(variant, node, node->Object, column);
    }
  }

  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    if(node->IsFolder())
    {
      if(column == CommonColumns::Name)
      {
        String folderName = variant.Get<String>();
        Status status;
        if(IsValidName(folderName,status))
        {
          node->Name = folderName;
          EventOnNode(node, Events::DataModified);
          return true;
        }
        else
        {
          DoNotifyWarning("Invalid folder name.",status.Message);
          EventOnNode(node, Events::DataModified);
          return false;
        }
      }
    }
    else
    {
      Policy::SetData(variant, node, node->Object, column);
      EventOnNode(node, Events::DataModified);
      return true;
    }

    return false;
  }

  bool Remove(DataEntry* dataEntry) override
  {
    /* Need to send command events instead of just removing it
      for ui prompts etc.
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    EraseNode(node, false);
    return true;
    */
    return false;
  }

  //------------------------------------------------------------

  //Tree Data Source Interface
  void Expand(DataEntry* dataEntry) override
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    Policy::Expand(this, node, node->Object);
  }

  void Collapse(DataEntry* dataEntry) override
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    Policy::Collaspe(this, node, node->Object);
  }

  bool IsExpandable(DataEntry* dataEntry) override
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    if(node->Flags.IsSet(NodeFlags::Folder))
      return true;
    else
      return Policy::IsExpandable(node->Object);
  }

  uint ChildCount(DataEntry* dataEntry) override
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    return node->ChildCount();
  }

  bool Move(DataEntry* dest, DataEntry* node, InsertMode::Type insertMode) override
  {
    TreeNodeType* destNode = (TreeNodeType*)dest;
    TreeNodeType* treeNode = (TreeNodeType*)node;
    TreeNodeType* newParent;
    uint where = -1;

    if(destNode->Flags.IsSet(NodeFlags::Folder))
      newParent = destNode;
    else
    {
      newParent = destNode->Parent;
      where = destNode->Index;
    }

    //Do not move to self
    if(newParent==treeNode)
      return false;

    TreeNodeType* oldParent = treeNode->Parent;

    oldParent->RemoveChild(treeNode);
    EventOnNode(treeNode, Events::DataRemoved);

    newParent->AddChild(treeNode, where);
    EventOnNode(newParent, Events::DataAdded);

    return true;
  }

  void Sort(DataEntry* dataEntry, StringParam column, bool flip) override
  {
    TreeNodeType* treeNode = (TreeNodeType*)dataEntry;
    Array<uint> indices;
    Array<String> strings;
    Array<TreeNodeType*> oldChildren;

    uint childCount = treeNode->Children.Size();
    indices.Resize(childCount);
    strings.Resize(childCount);
    oldChildren.Resize(childCount);

    // First, Insert all folders
    uint currIndex = 0;
    for(uint i = 0; i < childCount; ++i)
    {
      // Get the node
      TreeNodeType* child = treeNode->Children[i];

      // Insert if it's a folder
      if(child->IsFolder())
      {
        Any v;
        GetData(child, v, column);
        strings[currIndex] = v.ToString();
        indices[currIndex] = currIndex;
        oldChildren[currIndex] = child;
        ++currIndex;
      }
    }

    uint folderCount = currIndex;

    // Now Insert all other items
    for(uint i = 0; i < childCount; ++i)
    {
      // Get the node
      TreeNodeType* child = treeNode->Children[i];

      // Insert if it's not a folder
      if(!child->IsFolder())
      {
        Any v;
        GetData(child, v, column);
        strings[currIndex] = v.ToString();
        indices[currIndex] = currIndex;
        oldChildren[currIndex] = child;
        ++currIndex;
      }
    }
    
    Array<uint>::iterator folderEnd = indices.Begin() + folderCount;
    // Sort the folders
    Zero::Sort(Array<uint>::range(indices.Begin(), folderEnd), DataSorter(strings));
    // Sort the items
    Zero::Sort(Array<uint>::range(folderEnd, indices.End()), DataSorter(strings));

    //Rebuild
    for(uint i = 0; i < childCount; ++i)
    {
      treeNode->Children[i] = oldChildren[indices[i]];
      treeNode->Children[i]->Index = i;
    }

    EventOnNode(treeNode, Events::DataAdded);
  }

  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    TreeNodeType* node = (TreeNodeType*)dataEntry;
    if(index < node->ChildCount())
      return node->Children[index];
    return NULL;
  }

};

//A tree of non expandable nodes.
template<typename type>
struct ObjectTree
{
  static bool IsExpandable(type object)
  {
    return false;
  }

  template<typename Tree, typename EntryType>
  static void Expand(Tree* tree, EntryType* parent, type object)
  {
  }

  template<typename Tree, typename EntryType>
  static void Collaspe(Tree* tree, EntryType* parent, type object)
  {
  }

  template<typename EntryType>
  static void Erase(EntryType* entry, type object)
  {

  }
};


}
