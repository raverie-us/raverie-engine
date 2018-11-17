//////////////////////////////////////////////////////////////////////////
/// Authors: Chris Peters, Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------- Geometry Import
DeclareEnum3(Generation, None, Build, Import);
DeclareEnum6(GeometryProcessorCodes, NoContent, Success, Failed, LoadGraph, LoadTextures, LoadGraphAndTextures);
DeclareEnum3(ImageProcessorCodes, Success, Failed, Reload);
//--------------------------------------------------------- Physics Mesh Builder
DeclareEnum3(LoopingMode, Default, Once, Looping);
DeclareEnum2(PhysicsMeshType, PhysicsMesh, ConvexMesh);

}// namespace Zero
