///////////////////////////////////////////////////////////////////////////////
///
/// \file TextResource.hpp
/// Declaration of simple text resource.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// A Text resource is a generic resource for simple text data for game logic.
class TextBlock : public DocumentResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TextBlock();
  ~TextBlock();

  StringRange LoadTextData() override;
  void ReloadData(StringRange data) override;
  String GetFormat() override;

  //On save new
  void Save(StringParam filename) override;

  //Text stored on block;
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

}
