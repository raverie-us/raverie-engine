// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

DeclareEnum3(Generation, None, Build, Import);
DeclareEnum6(GeometryProcessorCodes, NoContent, Success, Failed, LoadGraph, LoadTextures, LoadGraphAndTextures);
DeclareEnum3(ImageProcessorCodes, Success, Failed, Reload);
DeclareEnum3(LoopingMode, Default, Once, Looping);
DeclareEnum2(PhysicsMeshType, PhysicsMesh, ConvexMesh);

} // namespace Raverie
