// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

/// Image content is 2d image data loaded from
/// image files. Used to generate textures, sprites, etc.
class ImageContent : public ContentComposition
{
public:
  ZilchDeclareType(ImageContent, TypeCopyMode::ReferenceType);
  ImageContent();

  void BuildContentItem(BuildOptions& options) override;

  bool mReload;
};

void BuildImageFileDialogFilters(Array<FileDialogFilter>& filters);

} // namespace Zero
