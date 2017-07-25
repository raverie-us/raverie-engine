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

  String outputFile = BuildString(Meshes[0].Name, ".mesh");
  String destFile = FilePath::Combine(options.OutputPath, outputFile);
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  if (CheckFileAndMeta(options, sourceFile, destFile))
    return true;

  return CheckToolFile(options, outputFile, "GeometryProcessor.exe");
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
  SerializeNameDefault(Meshes, Array<MeshEntry>());

  if (stream.GetMode() == SerializerMode::Loading)
  {
    //Legacy single mesh format
    String mName;
    ResourceId mResourceId = 0;

    SerializeNameDefault(mName, String());
    SerializeNameDefault(mResourceId, ResourceId(0));

    if (mResourceId != 0)
    {
      MeshEntry entry;
      entry.Name = mName;
      entry.mResourceId = mResourceId;

      Meshes.PushBack(entry);
    }
  }
}

void MeshBuilder::BuildListing(ResourceListing& listing)
{
  forRange(MeshEntry& entry, Meshes.All())
  {
    String output = BuildString(entry.Name, ".mesh");
    String name = entry.Name;
    listing.PushBack(ResourceEntry(0, "Mesh", name, output, entry.mResourceId, this->mOwner, this));
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
