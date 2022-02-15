// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(TextureLoaded);
}

ZilchDefineType(ContentPackage, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindFieldProperty(mName);
  ZilchBindFieldProperty(mAuthor);
  ZilchBindFieldProperty(mTags);
  ZilchBindFieldProperty(mDescription);
}

ContentPackage::ContentPackage()
{
  mSize = 0;
  mVersionBuilt = 0;
  mLocal = true;
  mPreview = nullptr;
  mRequest = AsyncWebRequest::Create();
}

ContentPackage::~ContentPackage()
{
  mRequest->Cancel();
}

void ContentPackage::operator=(const ContentPackage& rhs)
{
  mName = rhs.mName;
  mAuthor = rhs.mAuthor;
  mDate = rhs.mDate;
  mDescription = rhs.mDescription;
  mTags = rhs.mTags;
  mSize = rhs.mSize;
  mVersionBuilt = rhs.mVersionBuilt;
}

void ContentPackage::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mAuthor, String());
  SerializeNameDefault(mDate, String());
  SerializeNameDefault(mDescription, String());
  SerializeNameDefault(mTags, String());
  SerializeNameDefault(mSize, (u64)0);
  SerializeNameDefault(mVersionBuilt, (uint)9147);
}

void ContentPackage::LoadStreamedTexture(StringParam url)
{
  // Start the web request
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnWebResponse);
  request->mUrl = url;
  request->Run();
}

void ContentPackage::LoadLocalTexture(StringParam location)
{
  Image image;
  Status status;
  LoadImage(status, location, &image);

  if (status.Failed())
    return;

  mPreview = Texture::CreateRuntime();
  mPreview->Upload(image);
}

void ContentPackage::OnWebResponse(WebResponseEvent* e)
{
  if (e->mResponseCode != WebResponseCode::OK)
    return;

  String location = FilePath::Combine(GetTemporaryDirectory(), "StreamedImage.png");
  WriteStringRangeToFile(location, e->mData);

  LoadLocalTexture(location);

  Event eventToSend;
  GetDispatcher()->Dispatch(Events::TextureLoaded, &eventToSend);
}

} // namespace Zero
