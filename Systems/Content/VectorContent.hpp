///////////////////////////////////////////////////////////////////////////////
///
/// \file VectorContent.hpp
/// Declaration of Vector content classes.
/// 
/// Authors: Chris Peters
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Font content file are true type font files. Loading into
/// vector or bitmap formats.
class FontContent : public ContentComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  FontContent();
};

//Normal font builder just copies the ttf for direct loading.
class FontBuilder : public DirectBuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  FontBuilder();
  void Generate(ContentInitializer& initializer) override;
};

}
