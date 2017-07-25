///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentPackage.cpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(TextureLoaded);
}

//------------------------------------------------------------- Content Package
//******************************************************************************
ZilchDefineType(ContentPackage, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindFieldProperty(mName);
  ZilchBindFieldProperty(mAuthor);
  ZilchBindFieldProperty(mTags);
  ZilchBindFieldProperty(mDescription);
}

//******************************************************************************
ContentPackage::ContentPackage()
{
  mSize = 0;
  mVersionBuilt = 0;
  mLocal = true;
  mPreview = nullptr;
}

//******************************************************************************
ContentPackage::~ContentPackage()
{
  mRequest.Cancel();
}

//******************************************************************************
void ContentPackage::operator=(const ContentPackage& rhs)
{
  mName = rhs.mName;
  mAuthor = rhs.mAuthor;
  mDate = rhs.mDate;
  mDescription = rhs.mDescription;
  mTags = rhs.mTags;
  mSize = rhs.mSize;
  mVersionBuilt = rhs.mVersionBuilt;
  mTileSize = rhs.mTileSize;
}

//******************************************************************************
void ContentPackage::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mAuthor, String());
  SerializeNameDefault(mDate, String());
  SerializeNameDefault(mDescription, String());
  SerializeNameDefault(mTags, String());
  SerializeNameDefault(mSize, (uint)0);
  SerializeNameDefault(mVersionBuilt, (uint)9147);
  SerializeNameDefault(mTileSize, Vec2::cZero);
}

//******************************************************************************
void ContentPackage::LoadStreamedTexture(StringParam url)
{
  // Start the web request
  ConnectThisTo(&mRequest, Events::WebResponse, OnWebResponse);
  mRequest.mUrl = url;
  mRequest.Run();
}

//******************************************************************************
void ContentPackage::LoadLocalTexture(StringParam location)
{
  Image image;
  Status status;
  LoadFromPng(status, &image, location);

  if(status.Failed())
    return;

  mPreview = Texture::CreateRuntime();
  mPreview->Upload(image);
}

//******************************************************************************
void ContentPackage::OnWebResponse(WebResponseEvent* e)
{
  if(e->ResponseCode != WebResponseCode::OK)
    return;

  String location = FilePath::Combine(GetTemporaryDirectory(), "StreamedImage.png");
  WriteStringRangeToFile(location, e->Data);

  LoadLocalTexture(location);

  Event eventToSend;
  GetDispatcher()->Dispatch(Events::TextureLoaded, &eventToSend);
}

}//namespace Zero
