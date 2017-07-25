///////////////////////////////////////////////////////////////////////////////
///
/// \file SceneGraph.cpp
/// Declaration of the SceneGraph Resource class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------ SceneGraphMaterial
SceneGraphMaterial::SceneGraphMaterial()
{
  UsageCount = 0;
  LoadedMaterial = nullptr;
}

void SceneGraphMaterial::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeName(Attributes);
}

//------------------------------------------------------------------------ SceneGraphSource
ZilchDefineType(SceneGraphSource, builder, type)
{
}

SceneGraphSource::SceneGraphSource()
{
}

SceneGraphSource::~SceneGraphSource()
{
  SafeDelete(Root);
  DeleteObjectsInContainer(Materials);
}

void SceneGraphSource::Serialize(Serializer& stream)
{
  SerializeName(Materials);
  SerializeName(Root);
}

void SceneGraphSource::MapNames()
{
  forRange(SceneGraphMaterial* mat, Materials.All())
    MaterialsByName[mat->Name] = mat;
}

SceneGraphNode::SceneGraphNode()
{
}

SceneGraphNode::~SceneGraphNode()
{
  DeleteObjectsInContainer(Children);
}

void SceneGraphNode::Serialize(Serializer& stream)
{
  SerializeName(NodeName);

  SerializeName(Translation);
  SerializeName(Rotation);
  SerializeName(Scale);

  SerializeName(MeshName);
  SerializeName(SkeletonRootNodePath);
  SerializeName(PhysicsMeshName);

  SerializeName(IsSkeletonRoot);

  SerializeName(Materials);

  SerializeName(Attributes);

  SerializeName(Children);
}

}
