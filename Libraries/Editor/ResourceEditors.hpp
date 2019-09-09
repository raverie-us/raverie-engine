// MIT Licensed (see LICENSE.md).
#pragma once
namespace Zero
{
class Editor;

typedef void (*EditResourceFunction)(Editor*, Resource*);

/// Store resource editors
class ResourceEditors : public ExplicitSingleton<ResourceEditors, Object>
{
public:
  ZilchDeclareType(ResourceEditors, TypeCopyMode::ReferenceType);
  typedef HashMap<BoundType*, EditResourceFunction> ResourceEditorMap;
  ResourceEditorMap Editors;

  ResourceEditors();

  void FindResourceEditor(Resource* resource);
};

} // namespace Zero
