///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorSupport.cpp
/// Implementation of the Editor support classes EditorSpace and Selection.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
  RuntimeEditor* gRuntimeEditor = NULL;
  bool EditorDebugFeatures = false;
  bool DeveloperMode = false;
}

namespace SpecialCogNames
{
const String EditorCamera = "EditorCamera";
const String LevelSettings = "LevelSettings";
const String WorldAnchor = "WorldAnchor";
const String LevelGeometry = "LevelGeometry";
const String Main = "Main";
const String ViewCube = "ViewCube";
}

namespace CoreArchetypes
{
const String Game = "Game";
const String Space = "Space";
const String Transform = "Transform";
const String Empty = "Empty";
const String Sphere = "Sphere";
const String Cylinder = "Cylinder";
const String LevelSettings = "LevelSettings";
const String WorldAnchor = "WorldAnchor";
const String Camera = "Camera";
const String EditorCamera = "EditorCamera";
const String PreviewCamera = "PreviewCamera";
const String ObjectLink = "ObjectLink";
const String Default = "DefaultArchetype";
const String Sprite = "Sprite";
const String SpriteText = "SpriteText";
const String SpriteParticles = "SpriteParticles";
const String SplineParticleSystem = "SplineParticleSystem";
const String DirectionalLight = "DirectionalLight";
const String DirectionalLightShadows = "DirectionalLightShadows";
const String PointLight = "PointLight";
const String Cube = "Cube";
const String Grid = "Grid";
const String DefaultSpace = "DefaultSpace";
const String EmptyTile = "EmptyTile";
const String ViewCube = "ViewCube";
const String Wedge = "Wedge";
}


}
