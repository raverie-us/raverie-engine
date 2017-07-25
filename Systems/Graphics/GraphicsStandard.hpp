#pragma once

// Other projects
#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"
#include "Engine/EngineStandard.hpp"
#include "Meta/MetaStandard.hpp"
#include "SpatialPartition/SpatialPartitionStandard.hpp"
#include "ZilchShaders/ZilchShadersStandard.hpp"

namespace Zero
{

// Graphics library
class ZeroNoImportExport GraphicsLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(GraphicsLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Zero

#include "ForwardDeclarations.hpp"
#include "Enumerations.hpp"
#include "UtilityStructures.hpp"
#include "ResourceLists.hpp"

// No Dependencies
#include "Camera.hpp"
#include "FileElements.hpp"
#include "Font.hpp"
#include "GraphicalEntry.hpp"
#include "GraphicsRaycastProvider.hpp"
#include "MaterialBlock.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Particle.hpp"
#include "ParticleAnimator.hpp"
#include "ParticleEmitter.hpp"
#include "PerspectiveTransforms.hpp"
#include "PixelBuffer.hpp"
#include "Renderer.hpp"
#include "RenderGroup.hpp"
#include "RenderQueues.hpp"
#include "RenderSettings.hpp"
#include "RenderTarget.hpp"
#include "RenderTasks.hpp"
#include "Skeleton.hpp"
#include "SpriteSource.hpp"
#include "Texture.hpp"
#include "TextureData.hpp"
#include "TextureLoader.hpp"
#include "TextureUtilities.hpp"
#include "ViewportInterface.hpp"
#include "VisibilityFlag.hpp"
#include "ZilchFragment.hpp"
#include "ZilchShaderGenerator.hpp"

// Some Dependencies
#include "Atlas.hpp"
#include "FontPattern.hpp"
#include "MaterialFactory.hpp"
#include "ParticleEmitters.hpp"
#include "RendererThread.hpp"

// Base Graphicals
#include "Graphical.hpp"
#include "ParticleSystem.hpp"
#include "ParticleAnimators.hpp"

// Graphicals
#include "DebugGraphical.hpp"
#include "HeightMapModel.hpp"
#include "Model.hpp"
#include "SelectionIcon.hpp"
#include "SkinnedModel.hpp"
#include "Sprite.hpp"
#include "SpriteSystem.hpp"

#include "GraphicsSpace.hpp"

#include "GraphicsEngine.hpp"

// Deprecate
#include "Definition.hpp"
#include "Image.hpp"
#include "Text.hpp"
