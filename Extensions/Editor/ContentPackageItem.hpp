///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentPackage.hpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(TextureLoaded);
}

//------------------------------------------------------------- Content Package
class ContentPackage : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ContentPackage();
  ~ContentPackage();

  void operator=(const ContentPackage& rhs);

  void Serialize(Serializer& stream);

  void LoadStreamedTexture(StringParam url);
  void LoadLocalTexture(StringParam folder);

  /// Unique name of the content package.
  String mName;

  /// The author of the content package.
  String mAuthor;

  /// The date it was created / modified.
  String mDate;

  /// A short description of the content package.
  String mDescription;

  /// Comma delimited tags for searching content packages.
  String mTags;

  /// Size in bytes.
  uint mSize;

  /// Which version the package was built in (not the required version).
  uint mVersionBuilt;

  /// Dimensions of the tile widget.
  Vec2 mTileSize;

  bool mLocal;

  HandleOf<Texture> mPreview;

private:
  void OnWebResponse(WebResponseEvent* e);

  ThreadedWebRequest mRequest;
};

}//namespace Zero
