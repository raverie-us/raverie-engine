///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorImport.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ImportOptions;

DeclareBitField4(UpdateFlags, 
  // Update the archetype resource
  Archetype,
  // Update transforms in archetype
  Transforms,
  // Update assigned meshes
  Meshes,
  // Update and generate materials
  Materials
);

// Update a object 
void UpdateToContent(Cog* object, UpdateFlags::Type flags);

// Run Editor Side Importing on a resource package. 
// (Generation of Archetypes, Materials, etc)
void DoEditorSideImporting(ResourcePackage* package, ImportOptions* options);

}
