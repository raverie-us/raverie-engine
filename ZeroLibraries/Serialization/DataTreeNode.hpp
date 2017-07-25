///////////////////////////////////////////////////////////////////////////////
///
/// \file DataTreeNode.hpp
/// Declaration of the data tree used for serialization.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Memory{class Pool;}

// Forward Declarations
class DataNode;
class DataObject;
class DataTreeLoader;

// When given a node tree back from the inherit data callback, this specifies
// what to do with that new node (or lack of). The reason for this is to handle
// issues that pop up when looking up the inherited data.
// An example is when trying to resolve an Archetype, if the Archetype doesn't
// exist, we may want to return a fallback archetype, and disable patching (as
// the properties that were going to be patched were relevant only to the
// missing Archetype, not the default).
DeclareEnum4(PatchResolveMethod,
             // Patch the given node. This should be the default functionality.
             PatchNode,
             // Use the given node and ignore all patch nodes beneath it
             UseNode,
             // Patches the given node with properties and ignores addition and
             // removal of child nodes. This was added for when Archetypes
             // cannot be resolved (because they were removed) and we still
             // want to create a fallback object in its place. We want to patch
             // Transform properties so that it's in the same place, but
             // we don't want to add or remove Components as it may 
             //PatchPropertiesOnly,
             // Remove the node that was supposed to inherit
             RemoveNode,
             // Loading
             Error);

// Used to resolve inherited data files
typedef PatchResolveMethod::Enum (*ResolveInheritedData)(StringRange inheritId,
                                                         DataNode*& result);

//---------------------------------------------------------------------- Context
struct DataTreeContext
{
  DataTreeContext()
  {
     Error = false;
     PatchRequired = false;
  }

  bool Error;
  String Filename;
  String Message;
  bool PatchRequired;
  DataTreeLoader* Loader;
};

DeclareEnum4(ParseErrorCodes, 
             FileError,
             ParsingError, 
             StructureError,
             PatchError);

DeclareEnum5(PatchState, None, 
             // The value of this node was directly patched
             SelfPatched,
             // This node was locally added
             Added,
             // This node should be removed in the patching process
             ShouldRemove,
             // A child of this node was patched
             ChildPatched);

DeclareEnum2(DependencyAction,
             Discard, // The patch node should be ignored
             Add);    // The patch node should be added

DeclareEnum2(DataNodeType, Object, Value);

DeclareBitField5(DataNodeFlags, 
                 // If set, this node should be added to the final tree during patching.
                 LocallyAdded,
                 ChildOrderOverride,
                 Property,
                 Array,
                 Enumeration);

//-------------------------------------------------------------------- Data Node
// The data tree is a kind of generic tree data structure for storing data
// loaded from structured text files (xml, JSON, custom). 
/// Base class for all data tree nodes.
class DataNode
{
public:
  /// Intrusive link for the parent to use.
  IntrusiveLink(DataNode, link);

  /// Typedefs.
  typedef InList<DataNode> DataNodeList;

  /// Custom memory allocation.
  static Memory::Pool* sPool;
  static void* operator new(size_t size);
  static void operator delete(void* pMem, size_t size);

  /// Constructor / destructor.
  DataNode(DataNodeType::Enum nodeType, DataNode* parent);
  ~DataNode();

  /// Get the parent node in the tree.
  DataNode* GetParent();

  /// Attaches this node to the given parent node. Removes it from the old parent.
  void AttachTo(DataNode* newParent);
  
  /// Detaches ourself from our parent.
  void Detach();
  
  /// Detaches the old child, and attaches the new child in the same place as the old.
  void ReplaceChild(DataNode* oldChild, DataNode* newChild);

  /// Moves the given child in-front of the given location.
  void MoveChild(DataNode* child, DataNode* location);

  /// Deletes this node and removes it from the parent node.
  void Destroy();

  /// Finds the next available sibling in the tree.
  DataNode* NextSibling();
  uint GetNumberOfChildren();
  DataNode* GetFirstChild();
  DataNode* FindChildWithName(StringRange name);
  DataNode* ChildAtIndex(uint index);
  DataNodeList::range GetChildren();

  DataNode* FindChildWithTypeName(StringRange typeName);
  /// The 'foundDuplicate' is to notify whomever is using this that there
  /// were multiple objects with the same type name.
  DataNode* FindChildWithTypeName(StringRange typeName, StringRange name, bool& foundDuplicate);
  DataNode* FindChildWithUniqueNodeId(Guid childId);

  void AddAttribute(StringParam name, StringParam value);

  /// Returns whether or not this node or a child node has been patched.
  bool IsPatched();

  /// Whether or not this node is of a property type.
  bool IsProperty();
  bool HasChildProperties();

  /// Patches this based on the given tree's data.
  void Patch(DataNode* patchNode, DataTreeContext& c);
  void SetPatchStateRecursive(PatchState::Enum state);

  /// Places this node after the given sibling node. If null is passed in, it will place
  /// this node at the front of the children.
  void PlaceAfterSibling(DataNode* sibling);

  /// Flags.
  bool IsLocallyAdded();
  bool IsChildOrderOverride();
  bool IsArray();

  void ClearPatchState();

  /// Creates a deep copy of the entire tree.
  DataNode* Clone();

  void SaveToStream(Serializer& stream);

  /// The type of this node.
  DataNodeType::Enum mNodeType;

  /// Parent node in the tree.
  DataNode* mParent;

  /// The name of the field.
  String mPropertyName;

  /// The type of the field.
  String mTypeName;

  /// Value if DataNodeType::Value.
  String mTextValue;

  /// Children in the tree.
  uint mNumberOfChildren;
  DataNodeList mChildren;

  /// Attributes defined on Object nodes.
  DataAttributes mAttributes;

  /// Shows if / how this node was patched.
  PatchState::Enum mPatchState;

  BitField<DataNodeFlags::Enum> mFlags;

  /// Child nodes that were removed from a patch.
  struct RemovedNode
  {
    String mTypeName;
    Guid mUniqueNodeId;
  };
  Array<RemovedNode> mRemovedChildren;

  /// The id that this node was inherited from.
  String mInheritedFromId;

  /// A unique id (unique within its siblings) that's used in data patching
  /// to correctly resolve child nodes.
  Guid mUniqueNodeId;
};

DataNode* ReadDataSet(Status& status, StringRange data, StringParam source,
                      DataTreeLoader* loader, uint* fileVersion);

}//namespace Zero
