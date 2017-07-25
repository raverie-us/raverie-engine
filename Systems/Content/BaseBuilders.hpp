///////////////////////////////////////////////////////////////////////////////
///
/// \file BaseBuilders.hpp
///  Base Class Builders
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------- Direct Builder Component
///Direct builder is a 'mix in' class for making builder components.
class DirectBuilderComponent : public BuilderComponent
{
public:
  //Display name for this content.
  String Name;
  //Resource Id for this content.
  ResourceId mResourceId;
  //Extension of output resource.
  String Extension;
  //Loader of output resource.
  String LoaderType;
  //Order of output resource.
  uint Order;
  //Owner of output resource, if any.
  String ResourceOwner;

  DirectBuilderComponent(uint order, StringParam extension, StringParam loaderType)
    : Order(order), Extension(extension), LoaderType(loaderType)
  {}

  //BuilderComponent interface
  void Rename(StringParam newName) override;
  void BuildContent(BuildOptions& buildOptions) override;
  void Serialize(Serializer& stream) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildListing(ResourceListing& listing) override;
  void Generate(ContentInitializer& initializer) override;

  String GetResourceOwner() override { return ResourceOwner; }
  void SetResourceOwner(StringParam owner) override { ResourceOwner = owner; }

  //Helper functions
  String GetOutputFile();
  bool NeedsBuildingTool(BuildOptions& options, StringParam toolFile);
};

}
