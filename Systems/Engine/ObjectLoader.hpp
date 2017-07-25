///////////////////////////////////////////////////////////////////////////////
///
/// \file ObjectLoader.hpp
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
//class Status;
class ObjectState;
class CachedModifications;
class PropertyPath;

/* Example:

{
  ObjectLoader loader;
  loader.OpenFile("Enemy.data");

  CachedModifications modifications;
  loader.CacheModifications(&modifications);

  // Create Cog
  Cog* enemy = CreateFromLoader(loader);

  modifications.ApplyModificationsToObject(enemy);
}

*/

//------------------------------------------------------------------------------------ Object Loader
/// This loader handles loading data tree patches specific to Zero Objects
/// (i.e. Cogs and Resources). It resolves both data inheritance and
/// dependency checking when patching the data tree.
class ObjectLoader : public DataTreeLoader
{
public:
  /// Adds all modifications to the given object to the LocalModifications
  void RecordModifications(Object* rootObject);

  void CacheModifications(CachedModifications* modifications);

private:
  /// DataTreeLoader Interface.
  PatchResolveMethod::Enum ResolveInheritedData(StringRange inheritId,
                                                DataNode*& result) override;
  DependencyAction::Enum ResolveDependencies(DataNode* parent,
                                             DataNode* newChild,
                                             DataNode** toReplace,
                                             Status& status) override;
};

//----------------------------------------------------------------------------- Cached Modifications
/// Given a data tree, this builds local modifications that can be applied to a given Object
/// at a later time.
class CachedModifications
{
public:
  /// Forward declarations.
  struct ObjectNode;

  /// Constructor / Destructor.
  CachedModifications();
  ~CachedModifications();

  /// Builds local modifications from the give data tree. The object states can then be applied
  /// to objects later once they have been allocated. This is primarily used to cache the
  /// object states for Archetypes so that the information can be quickly cloned.
  void Cache(DataNode* root);
  void Cache(Object* object);
  void Combine(CachedModifications& modifications);

  void ApplyModificationsToObject(Object* object, bool combine = false);
  void ApplyModificationsToChildObject(Object* rootObject, Object* childObject, bool combine);

  /// Any modifications that are stored on the object and the given cached modifications will
  /// be stored on this.
  void StoreOverlappingModifications(Object* object, ObjectNode* cachedNode);

  ObjectNode* FindChildNode(Object* rootObject, Object* childObject);

  void Clear();
  bool Empty();

  //------------------------------------------------------------------------------------ Object Node
  struct ObjectNode
  {
    /// Constructor / Destructor.
    ObjectNode();
    ~ObjectNode();

    ObjectState* GetObjectState();
    ObjectNode* FindChild(Object* child);
    ObjectNode* Clone();
    void Combine(ObjectNode* toCombine);

    /// All modifications for this node.
    ObjectState* mState;

    typedef HashMap<ObjectState::ChildId, ObjectNode*> ChildMap;
    ChildMap mChildren;
  };

private:
  void ApplyModificationsToObjectInternal(Object* object, ObjectNode* objectNode, bool combine);
  ObjectNode* StoreOverlappingModificationsInternal(Object* object, ObjectNode* node);

  void ExtractInternal(DataNode* dataNode, ObjectNode* objectNode, PropertyPath& path);
  ObjectNode* ExtractInternal(Object* object);

  ObjectNode* mRootObjectNode;
};

}//namespace Zero
