// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(MeshBuilder, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::CallSetDefaults);
  RaverieBindDependency(GeometryContent);

  RaverieBindFieldProperty(mGenerateSmoothNormals);
  RaverieBindFieldProperty(mSmoothingAngleDegreesThreshold);
  RaverieBindFieldProperty(mGenerateTangentSpace);
  RaverieBindFieldProperty(mInvertUvYAxis);
  RaverieBindFieldProperty(mFlipWindingOrder);
  RaverieBindFieldProperty(mFlipNormals);
}

MeshBuilder::MeshBuilder() :
    mCombineMeshes(false),
    mGenerateSmoothNormals(false),
    mSmoothingAngleDegreesThreshold(30.f),
    mGenerateTangentSpace(true),
    mInvertUvYAxis(false),
    mFlipWindingOrder(false),
    mFlipNormals(false)
{
}

bool MeshBuilder::NeedsBuilding(BuildOptions& options)
{
  if (Meshes.Empty())
    return true;

  String outputFile = BuildString(Meshes[0].mName, ".mesh");
  String destFile = FilePath::Combine(options.OutputPath, outputFile);
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  return CheckFileAndMeta(options, sourceFile, destFile);
}

void MeshBuilder::Generate(ContentInitializer& initializer)
{
}

void MeshBuilder::Serialize(Serializer& stream)
{
  // SerializeNameDefault(mCombineMeshes, false);
  SerializeNameDefault(mGenerateSmoothNormals, false);
  SerializeNameDefault(mSmoothingAngleDegreesThreshold, 30.f);
  SerializeNameDefault(mGenerateTangentSpace, true);
  SerializeNameDefault(mInvertUvYAxis, false);
  SerializeNameDefault(mFlipWindingOrder, false);
  SerializeNameDefault(mFlipNormals, false);
  SerializeNameDefault(Meshes, Array<GeometryResourceEntry>());
}

void MeshBuilder::BuildListing(ResourceListing& listing)
{
  forRange (GeometryResourceEntry& entry, Meshes.All())
  {
    String output = BuildString(entry.mName, ".mesh");
    listing.PushBack(ResourceEntry(0, "Mesh", entry.mName, output, entry.mResourceId, this->mOwner, this));
  }
}

VertexAttribute::VertexAttribute(VertexSemantic::Enum semantic, VertexElementType::Enum type, byte count, byte offset) :
    mSemantic(semantic),
    mType(type),
    mCount(count),
    mOffset(offset)
{
}

} // namespace Raverie
