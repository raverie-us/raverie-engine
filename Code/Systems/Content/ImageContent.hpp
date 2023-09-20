// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Image content is 2d image data loaded from
/// image files. Used to generate textures, sprites, etc.
class ImageContent : public ContentComposition
{
public:
  RaverieDeclareType(ImageContent, TypeCopyMode::ReferenceType);
  ImageContent();

  void BuildContentItem(BuildOptions& options) override;

  bool mReload;
};

void BuildImageFileDialogFilters(Array<FileDialogFilter>& filters);

} // namespace Raverie
