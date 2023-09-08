// MIT Licensed (see LICENSE.md).
#pragma once

#include "Foundation/Common/CommonStandard.hpp"
#include "Systems/Engine/EngineStandard.hpp"
#include "Foundation/SpatialPartition/SpatialPartitionStandard.hpp"

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

} // namespace Zero

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
#include "AudioFileEncoder.hpp"
#include "AudioContent.hpp"
#include "ImportOptions.hpp"
#include "ImageContent.hpp"
#include "SpriteBuilder.hpp"
#include "TextContent.hpp"
#include "SupportComponents.hpp"
#include "RichAnimation.hpp"
#include "GeometryContent.hpp"
#include "BinaryContent.hpp"
#include "MeshBuilder.hpp"
#include "ResourceLoaderTemplates.hpp"
