// MIT Licensed (see LICENSE.md).
#pragma once

#include "ContentStandard.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

// GeometryProcessor is privately included because it directly references assimp
#include "GeometryProcessorDataStructures.hpp"
#include "GeometryUtility.hpp"
#include "VertexDescriptionBuilder.hpp"
#include "AnimationProcessor.hpp"
#include "ArchetypeProcessor.hpp"
#include "GeometryImporter.hpp"
#include "MeshProcessor.hpp"
#include "PhysicsMeshProcessor.hpp"
#include "SkeletonProcessor.hpp"
#include "TextureProcessor.hpp"
#include "PivotProcessor.hpp"

// ImageProcessor is privately included because it directly references nvtt
#include "nvtt/nvtt.h"

#include "CubemapProcessing.hpp"
#include "TextureImporter.hpp"
