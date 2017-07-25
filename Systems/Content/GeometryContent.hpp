///////////////////////////////////////////////////////////////////////////////
///
/// \file GeometryContent.hpp
/// Declaration of the Geometry content classes.
/// 
/// Authors: Chris Peters
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ContentComposition.hpp"
#include "ImportOptions.hpp"
#include "BaseBuilders.hpp"
#include "ContentEnumerations.hpp"
#include "MeshEntry.hpp"

namespace Zero
{

//-------------------------------------------------------------- Geometry Import

/// General import settings for Geometry scene.
class GeometryImport : public ContentComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream)  override;
  void Generate(ContentInitializer& initializer) override;

  void ComputeTransforms();

  Vec3 mOriginOffset;
  float mScaleFactor;
  bool mChangeBasis;
  BasisType::Enum mXBasisTo;
  BasisType::Enum mYBasisTo;
  BasisType::Enum mZBasisTo;

  Mat4 mTransform;
  Mat3 mNormalTransform;
  Mat3 mChangeOfBasis;
};

//--------------------------------------------------------- Physics Mesh Builder

//Build Collision Meshes on component
class PhysicsMeshBuilder : public BuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PhysicsMeshBuilder(){};

  void SetMeshBuilt(PhysicsMeshType::Enum type);
  PhysicsMeshType::Enum GetMeshBuilt();

  /// The type of mesh to make 
  PhysicsMeshType::Enum MeshBuilt;

  Array<MeshEntry> Meshes;

  //BuilderComponent Interface
  bool NeedsBuilding(BuildOptions& options) override;
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void BuildListing(ResourceListing& listing) override;
  String GetOutputFile(uint index);
};

//--------------------------------------------------------------- Animation Clip
class AnimationClip : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AnimationClip();
  void Serialize(Serializer& stream);
  void SetDefaults();

  String Name;
  ResourceId mResourceId;

  int mStartFrame;
  int mEndFrame;
  LoopingMode::Enum mLoopingMode;
};

//------------------------------------------------------------ Animation Builder
class AnimationBuilder : public DirectBuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AnimationBuilder()
    : DirectBuilderComponent(10, ".animset.data", "AnimationSet")
  {
  }

  ~AnimationBuilder();

  Array<AnimationClip> mClips;

  //BuilderComponent Interface
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildListing(ResourceListing& listing) override;
  void Serialize(Serializer& stream) override;
  void Generate(ContentInitializer& initializer) override;
};

//GeneratedArchetype
class GeneratedArchetype : public ContentComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GeneratedArchetype();

  bool NeedToBuildArchetype(ContentItem* contentItem);
  ResourceId mResourceId;
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
};
//------------------------------------------------------------------------ Texture Content
class TextureEntry : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream)
  {
    SerializeName(mFullFilePath);
    SerializeName(mResourceId);
  }

  String mFullFilePath;
  ResourceId mResourceId;
};

// Textures imported from mesh files with embedded textures
class TextureContent : public ContentComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream)  override;
  void Generate(ContentInitializer& initializer) override;

  Array<TextureEntry> mTextures;
};

//------------------------------------------------------------------------ Geometry Content
class GeometryContent : public ContentComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GeometryContent();
  GeometryContent(StringParam inputFilename);
  String GetName();
  //Content Item Interface
  void BuildContent(BuildOptions& options) override;
  GeometryContent(ContentInitializer& initializer);
};

void AddGeometryFileFilters(ResourceManager* manager);

}//namespace Zero
