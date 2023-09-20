// MIT Licensed (see LICENSE.md).
#pragma once
#include "ContentComposition.hpp"
#include "ImportOptions.hpp"
#include "BaseBuilders.hpp"
#include "ContentEnumerations.hpp"

namespace Raverie
{

/// General import settings for Geometry scene.
class GeometryImport : public ContentComponent
{
public:
  RaverieDeclareType(GeometryImport, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;
  void Generate(ContentInitializer& initializer) override;

  void ComputeTransforms();
  bool mCollapsePivots;
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

class GeometryResourceEntry : public Object
{
public:
  RaverieDeclareType(GeometryResourceEntry, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream);
  void SetDefaults();

  bool operator==(const GeometryResourceEntry& other);

  String mName;
  ResourceId mResourceId;
};

// Build Collision Meshes on component
class PhysicsMeshBuilder : public BuilderComponent
{
public:
  RaverieDeclareType(PhysicsMeshBuilder, TypeCopyMode::ReferenceType);

  PhysicsMeshBuilder(){};

  void SetMeshBuilt(PhysicsMeshType::Enum type);
  PhysicsMeshType::Enum GetMeshBuilt();

  /// The type of mesh to make
  PhysicsMeshType::Enum MeshBuilt;

  Array<GeometryResourceEntry> Meshes;

  // BuilderComponent Interface
  bool NeedsBuilding(BuildOptions& options) override;
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void BuildListing(ResourceListing& listing) override;
  String GetOutputFile(uint index);
};

class AnimationClip : public Object
{
public:
  RaverieDeclareType(AnimationClip, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream);
  void SetDefaults();

  String mName;
  int mStartFrame;
  int mEndFrame;
  int mAnimationIndex;
};

class AnimationBuilder : public DirectBuilderComponent
{
public:
  RaverieDeclareType(AnimationBuilder, TypeCopyMode::ReferenceType);

  AnimationBuilder() : DirectBuilderComponent(10, ".animset.data", "AnimationSet")
  {
  }

  Array<AnimationClip> mClips;
  Array<GeometryResourceEntry> mAnimations;

  // BuilderComponent Interface
  void Serialize(Serializer& stream) override;
  void Generate(ContentInitializer& initializer) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildListing(ResourceListing& listing) override;
};

// GeneratedArchetype
class GeneratedArchetype : public ContentComponent
{
public:
  RaverieDeclareType(GeneratedArchetype, TypeCopyMode::ReferenceType);

  GeneratedArchetype();

  bool NeedToBuildArchetype(ContentItem* contentItem);
  ResourceId mResourceId;
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
};

// Texture Content
// Textures imported from mesh files with embedded textures
class TextureContent : public ContentComponent
{
public:
  RaverieDeclareType(TextureContent, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;
  void Generate(ContentInitializer& initializer) override;

  Array<GeometryResourceEntry> mTextures;
};

// Geometry Content
class GeometryContent : public ContentComposition
{
public:
  RaverieDeclareType(GeometryContent, TypeCopyMode::ReferenceType);

  GeometryContent();
  GeometryContent(StringParam inputFilename);
  String GetName();
  // Content Item Interface
  void BuildContentItem(BuildOptions& options) override;
  GeometryContent(ContentInitializer& initializer);
};

void AddGeometryFileFilters(ResourceManager* manager);

} // namespace Raverie
