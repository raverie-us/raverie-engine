//////////////////////////////////////////////////////////////////////////
/// Authors: Chris Peters, Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

 
ZilchDefineType(MeshBuilder, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);
  ZeroBindDependency(GeometryContent);

  ZilchBindFieldProperty(mGenerateSmoothNormals);
  ZilchBindFieldProperty(mSmoothingAngleDegreesThreshold);
  ZilchBindFieldProperty(mGenerateTangentSpace);
  ZilchBindFieldProperty(mInvertUvYAxis);
  ZilchBindFieldProperty(mFlipWindingOrder);
  ZilchBindFieldProperty(mFlipNormals);
}

MeshBuilder::MeshBuilder()
  : mCombineMeshes(false),
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
  //SerializeNameDefault(mCombineMeshes, false);
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
  forRange(GeometryResourceEntry& entry, Meshes.All())
  {
    String output = BuildString(entry.mName, ".mesh");
    listing.PushBack(ResourceEntry(0, "Mesh", entry.mName, output, entry.mResourceId, this->mOwner, this));
  }
}

VertexAttribute::VertexAttribute(VertexSemantic::Enum semantic, VertexElementType::Enum type, byte count, byte offset)
  : mSemantic(semantic),
    mType(type),
    mCount(count),
    mOffset(offset)
{

}

}// namespace Zero
