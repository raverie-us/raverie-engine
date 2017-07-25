///////////////////////////////////////////////////////////////////////////////
///
/// \file ImageContent.hpp
/// Declaration of the Image content class.
/// 
/// Authors: Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------- Image Content
/// Image content is 2d image data loaded from
/// image files. Used to generate textures, sprites, etc.
class ImageContent : public ContentComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ImageContent();

  void BuildContent(BuildOptions& options) override;

  bool mReload;
};

}//namespace Zero
