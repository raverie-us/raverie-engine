///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentStandard.hpp
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Math/MathStandard.hpp"
#include "Platform/PlatformStandard.hpp"
#include "Engine/EngineStandard.hpp"

namespace Zero
{
// Forward declarations
class ContentLibrary;
class ContentItem;
class ContentComponent;
class BuildOptions;
class ContentComposition;

// Content library
class ZeroNoImportExport ContentMetaLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(ContentMetaLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

}//namespace Zero

// Our includes
#include "FileExtensionManager.hpp"
#include "ContentItem.hpp"
#include "ContentLibrary.hpp"
#include "BuildOptions.hpp"
#include "ContentSystem.hpp"
#include "ContentUtility.hpp"
#include "ContentComposition.hpp"
#include "DataContent.hpp"
#include "TagsContent.hpp"
#include "BaseBuilders.hpp"
#include "ZilchPluginContent.hpp"
#include "VectorContent.hpp"
#include "TextureBuilder.hpp"
#include "ImportOptions.hpp"
#include "ImageContent.hpp"
#include "SpriteBuilder.hpp"
#include "TextContent.hpp"
#include "SupportComponents.hpp"
#include "RichAnimation.hpp"
#include "GeometryContent.hpp"
#include "AudioContent.hpp"
#include "BinaryContent.hpp"
#include "MeshEntry.hpp"
#include "MeshBuilder.hpp"
