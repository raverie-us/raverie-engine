// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

cstr errorMessage = "Need builder type for binary files."
                    " Since file is binary a loader must be provided.";

ContentItem* MakeBinaryContent(ContentInitializer& initializer)
{
  // any extensions?
  if (initializer.Extension == "dds")
    initializer.BuilderType = "TextureDds";

  if (initializer.Extension == "convexmesh")
    initializer.BuilderType = "ConvexMesh";
  if (initializer.Extension == "multiconvexmesh")
    initializer.BuilderType = "MultiConvexMesh";

  if (initializer.Extension == "cubetex")
    initializer.BuilderType = "TextureCube";

  // if(initializer.Extension == "hdr")
  // initializer.BuilderType = "TextureHdr";

  if (initializer.BuilderType.Empty())
  {
    ErrorIf(initializer.BuilderType.Empty(), errorMessage);
    initializer.Success = false;
    initializer.Message = errorMessage;
    return nullptr;
  }

  // Make the content item
  BinaryContent* content = new BinaryContent();
  content->Filename = initializer.Filename;

  // Make the builder component
  BinaryBuilder* builder = new BinaryBuilder();
  builder->Generate(initializer);

  // Add the builder
  content->AddComponent(builder);

  // Done
  return content;
}

ZilchDefineType(BinaryContent, builder, type)
{
}

BinaryContent::BinaryContent()
{
  EditMode = ContentEditMode::ResourceObject;
}

ZilchDefineType(BinaryBuilder, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);
  ZeroBindDependency(BinaryContent);
}

void BinaryBuilder::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeName(mResourceId);
  SerializeName(LoaderType);
  SerializeName(Version);
  SerializeNameDefault(FilterTag, String());
  SerializeNameDefault(ResourceOwner, String());
}

void BinaryBuilder::Generate(ContentInitializer& initializer)
{
  // Generate a new resource id
  if (initializer.AddResourceId == 0)
    mResourceId = GenerateUniqueId64();
  else
    mResourceId = initializer.AddResourceId;

  Name = initializer.Name;

  LoaderType = initializer.BuilderType;
  Version = 0;
  ResourceOwner = initializer.ResourceOwner;
}

void CreateBinaryContent(ContentSystem* system)
{
  AddContentComponent<BinaryBuilder>(system);
  AddContent<BinaryContent>(system);

  ContentTypeEntry binary(ZilchTypeId(BinaryContent), MakeBinaryContent);
  system->CreatorsByExtension["bin"] = binary;
  system->CreatorsByExtension["convexmesh"] = binary;
  system->CreatorsByExtension["multiconvexmesh"] = binary;
  system->CreatorsByExtension["physmesh"] = binary;
  system->CreatorsByExtension["cubetex"] = binary;
  // system->CreatorsByExtension["hdr"] = binary;
}

} // namespace Zero
