// MIT Licensed (see LICENSE.md).

#pragma once

// Other projects
#include "Foundation/Common/CommonStandard.hpp"
#include "Renderer/RendererBase/RendererBaseStandard.hpp"
#include "Systems/Engine/EngineStandard.hpp"
#include "Foundation/Meta/MetaStandard.hpp"
#include "Foundation/SpatialPartition/SpatialPartitionStandard.hpp"
#include "Extensions/RaverieShaders/RaverieShadersStandard.hpp"

namespace Raverie
{

// Graphics library
class GraphicsLibrary : public Raverie::StaticLibrary
{
public:
  RaverieDeclareStaticLibraryInternals(GraphicsLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Raverie

#include "ForwardDeclarations.hpp"
#include "UtilityStructures.hpp"
#include "ResourceLists.hpp"

// No Dependencies
#include "Camera.hpp"
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
#include "RenderGroup.hpp"
#include "RendererGlobal.hpp"
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
#include "RaverieFragment.hpp"
#include "RaverieShaderGlslBackend.hpp"
#include "RaverieShaderGenerator.hpp"

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
