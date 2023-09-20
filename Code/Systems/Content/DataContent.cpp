// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "DataContent.hpp"
#include "Platform/FileSystem.hpp"
#include "ContentLibrary.hpp"
#include "Foundation/Serialization/Simple.hpp"
#include "ContentSystem.hpp"
#include "RichAnimation.hpp"
#include "Platform/FilePath.hpp"

namespace Raverie
{

ContentItem* MakeDataContent(ContentInitializer& initializer)
{
  String fullPath = FilePath::Combine(initializer.Library->SourcePath, initializer.Filename);

  // What type is in the file so we know what builder type
  String loaderType = GetTypeInFile(fullPath);

  DataContent* content = new DataContent();

  content->Filename = initializer.Filename;

  ContentComponent* builder = nullptr;
  if (loaderType == "RichAnimation")
  {
    builder = new RichAnimationBuilder();
  }
  else
  {
    DataBuilder* dataBuilder = new DataBuilder();
    dataBuilder->LoaderType = loaderType;
    builder = dataBuilder;
  }

  builder->Generate(initializer);
  content->AddComponent(builder);

  return content;
}

RaverieDefineType(DataContent, builder, type)
{
}

DataContent::DataContent()
{
  EditMode = ContentEditMode::ResourceObject;
}

RaverieDefineType(DataBuilder, builder, type)
{
  RaverieBindDependency(DataContent);
}

String DataBuilder::GetOutputFile()
{
  return mOwner->Filename;
}

void DataBuilder::Generate(ContentInitializer& initializer)
{
  // For DataBuilders a ResourceId can be provided
  if (initializer.AddResourceId == 0)
    mResourceId = GenerateUniqueId64();
  else
    mResourceId = initializer.AddResourceId;

  Name = initializer.Name;

  Version = 0;
}

void DataBuilder::Serialize(Serializer& stream)
{
  SerializeNameDefault(Name, String());
  SerializeNameDefault(mResourceId, ResourceId(0));
  SerializeNameDefault(LoaderType, String());
  SerializeNameDefault(Version, (uint)0);
}

void DataBuilder::Rename(StringParam newName)
{
  Name = newName;
}

bool DataBuilder::NeedsBuilding(BuildOptions& options)
{
  String destFile = FilePath::Combine(options.OutputPath, GetOutputFile());
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  return CheckFileMetaAndSize(options, sourceFile, destFile);
}

void DataBuilder::BuildContent(BuildOptions& buildOptions)
{
  String destFile = FilePath::Combine(buildOptions.OutputPath, GetOutputFile());
  String sourceFile = FilePath::Combine(buildOptions.SourcePath, mOwner->Filename);
  bool fileCopied = CopyFile(destFile, sourceFile);

  if (!fileCopied)
  {
    buildOptions.Failure = true;
    buildOptions.Message = String::Format("Failed to copy data file %s to %s", sourceFile.c_str(), destFile.c_str());
  }

  SetFileToCurrentTime(destFile);
}

void DataBuilder::BuildListing(ResourceListing& listing)
{
  String destFile = GetOutputFile();

  uint order = Z::gContentSystem->LoadOrderMap.FindValue(LoaderType, 10);

  listing.PushBack(ResourceEntry(order, LoaderType, Name, destFile, mResourceId, this->mOwner, this));
}

void CreateDataContent(ContentSystem* system)
{
  AddContentComponent<DataBuilder>(system);
  AddContentComponent<RichAnimationBuilder>(system);
  AddContent<DataContent>(system);

  system->CreatorsByExtension["data"] = ContentTypeEntry(RaverieTypeId(DataContent), MakeDataContent);
}

} // namespace Raverie
