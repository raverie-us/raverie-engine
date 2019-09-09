// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

bool NeedToBuild(StringParam source, StringParam destination);
bool NeedToBuild(BuildOptions& options, StringParam source, StringParam destination);

/// Check the file modified time of the meta file and output file.
bool CheckFileAndMeta(BuildOptions& options, StringParam sourceFile, StringParam destFile);

/// Check the output file against the tool file.
bool CheckToolFile(BuildOptions& options, StringParam outputFile, StringParam toolFile);

/// Check the file modified time and file size. If the source file is newer or a
/// different size then the file needs to be built.
bool CheckFileMetaAndSize(BuildOptions& options, StringParam sourceFile, StringParam destFile);

template <typename type>
ContentItem* MakeContentItemFromFile(ContentInitializer& initializer)
{
  return new type(initializer);
}

template <typename type>
ContentItem* MakeContentItemT(ContentInitializer& initializer)
{
  return new type();
}

template <typename type>
ContentComponent* MakeContentComponentT()
{
  return new type();
}

template <typename type>
void AddContentComponent(ContentSystem* system)
{
  // METAREFACTOR - replace with generic meta composition
  BoundType* contentType = ZilchTypeId(type);
  system->ComponentFactory.Creators[contentType->Name] = MakeContentComponentT<type>;
  system->ContentComponents[contentType->Name] = contentType;
  // contentType->CreateObject = CreateObjectFunctionTemplate<type>;
}

template <typename type>
void AddContent(ContentSystem* system)
{
  // METAREFACTOR - replace with generic meta composition
  BoundType* contentType = ZilchTypeId(type);
  system->Creators[contentType->Name] = ContentTypeEntry(contentType, MakeContentItemT<type>);
}

// Compression Level
DeclareEnum4(ResourceCompression, None, Low, Medium, High);

} // namespace Zero
