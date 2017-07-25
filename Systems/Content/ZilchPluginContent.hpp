///////////////////////////////////////////////////////////////////////////////
///
/// \file ZilchPluginContent.hpp
/// 
/// Authors: Trevor Sundberg
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
ContentItem* MakeZilchPluginContent(ContentInitializer& initializer);

class ZilchPluginBuilder : public DataBuilder
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ZilchPluginBuilder();

  ResourceId mSharedLibraryResourceId;

  void Rename(StringParam newName) override;
  
  // Gets a platform and Zero revision dependent extension (ends with zilchPlugin)
  static String GetSharedLibraryPlatformName();
  static String GetSharedLibraryPlatformBuildName();
  static String GetSharedLibraryExtension(bool includeDot);

  String GetSharedLibraryFileName();
  
  void Serialize(Serializer& stream) override;
  void Generate(ContentInitializer& initializer) override;
  void BuildContent(BuildOptions& buildOptions) override;
  void BuildListing(ResourceListing& listing) override;
};


void CreateZilchPluginContent(ContentSystem* system);
ContentItem* MakeZilchPluginContent(ContentInitializer& initializer);

}//namespace Zero
