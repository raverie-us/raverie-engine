// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#pragma once

// Zero
#include "Content/ContentStandard.hpp"
#include "SpatialPartition/SpatialPartitionStandard.hpp"

// Assimp
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

// Assimp conversion
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
// Pivot needs to come after skeleton processor for constant variable access
#include "PivotProcessor.hpp"
