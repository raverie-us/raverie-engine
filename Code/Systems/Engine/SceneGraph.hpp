// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Material;

// Material for Scene Graph
class SceneGraphMaterial
{
public:
  SceneGraphMaterial();

  uint UsageCount;
  // Name of material
  String Name;
  // Attributes of material
  HashMap<String, String> Attributes;

  Material* LoadedMaterial;
  void Serialize(Serializer& stream);
};

// Node in scene graph.
class SceneGraphNode
{
public:
  String NodeName;
  Vec3 Translation;
  Quat Rotation;
  Vec3 Scale;

  SceneGraphNode();
  ~SceneGraphNode();

  // Mesh used for this Node
  String MeshName;
  String SkeletonRootNodePath;
  String PhysicsMeshName;

  // Is this node a skeleton root
  bool IsSkeletonRoot;

  // Materials use for this object
  Array<String> Materials;
  // Attributes of Node
  HashMap<String, String> Attributes;
  // Child Nodes
  Array<SceneGraphNode*> Children;

  void Serialize(Serializer& stream);
};

// Scene graph resource is used to load scene graphs from external
// resources/content. Used during content importing.
class SceneGraphSource
{
public:
  RaverieDeclareType(SceneGraphSource, TypeCopyMode::ReferenceType);

  String Name;

  SceneGraphSource();
  ~SceneGraphSource();

  // Root of Scene Graph
  SceneGraphNode* Root;

  // Materials in Scene Graph
  Array<SceneGraphMaterial*> Materials;

  HashMap<String, SceneGraphMaterial*> MaterialsByName;

  void MapNames();

  void Serialize(Serializer& stream);
};

} // namespace Raverie
