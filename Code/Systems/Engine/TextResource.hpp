// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// A Text resource is a generic resource for simple text data for game logic.
class TextBlock : public DocumentResource
{
public:
  RaverieDeclareType(TextBlock, TypeCopyMode::ReferenceType);

  TextBlock();
  ~TextBlock();

  StringRange LoadTextData() override;
  void ReloadData(StringRange data) override;
  String GetFormat() override;

  // On save new
  void Save(StringParam filename) override;

  // Text stored on block;
  String GetText();
  void SetText(StringParam newText);

  String Text;
};

/// The manager for text resources.
class TextBlockManager : public ResourceManager
{
public:
  DeclareResourceManager(TextBlockManager, TextBlock);

  TextBlockManager(BoundType* resourceType);
};

} // namespace Raverie
