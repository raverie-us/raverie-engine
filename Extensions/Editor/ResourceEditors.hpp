///////////////////////////////////////////////////////////////////////////////
///
/// \file ResourceEditors.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
namespace Zero
{
class Editor;

typedef void (*EditResourceFunction)(Editor*,Resource*);

/// Store resource editors
class ResourceEditors : public ExplicitSingleton<ResourceEditors, Object>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef HashMap<BoundType*, EditResourceFunction> ResourceEditorMap;
  ResourceEditorMap Editors;

  ResourceEditors();

  void FindResourceEditor(Resource* resource);
};

}
