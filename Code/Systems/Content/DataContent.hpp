// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// DataContent item. Data content is a content item that is stored in
/// the engine's text data format. These files Contains many resource types
/// and are usually edited by the editor directly.
class DataContent : public ContentComposition
{
public:
  RaverieDeclareType(DataContent, TypeCopyMode::ReferenceType);

  DataContent();
  DataContent(ContentInitializer& initializer);
};

/// Data builder content item. Data content builder basically just copies
/// the data file out. It also Contains the 'type' of what is in the file
/// to determine what loader to use.
class DataBuilder : public BuilderComponent
{
public:
  RaverieDeclareType(DataBuilder, TypeCopyMode::ReferenceType);

  String Name;
  ResourceId mResourceId;
  String LoaderType;
  uint Version;

  String GetOutputFile();

  // BuilderComponent Interface
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  void Rename(StringParam newName) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildContent(BuildOptions& buildOptions) override;
  void BuildListing(ResourceListing& listing) override;
};

} // namespace Raverie
