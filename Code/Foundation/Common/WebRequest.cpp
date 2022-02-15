// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

String WebRequest::GetBoundary()
{
  static const String cBoundary("----------" + GetOrganizationApplicationName() + GetGuidString() + "$");
  return cBoundary;
}

String WebRequest::GetContentTypeHeader()
{
  String contentType = BuildString("Content-Type:multipart/form-data; boundary=", GetBoundary(), "\r\n");

  return contentType;
}

String WebRequest::GetPostDataWithBoundaries()
{
  String boundary = GetBoundary();
  StringBuilder post;

  forRange (WebPostData& postData, mPostData)
  {
    post.Append("--");
    post.Append(boundary);
    post.Append("\r\n");
    post.Append("Content-Disposition: form-data; name=\"");
    post.Append(postData.mName);
    post.Append("\"");

    if (!postData.mFileName.Empty())
    {
      post.Append("; filename=\"");
      post.Append(postData.mFileName);
      post.Append("\"");
    }

    post.Append("\r\n\r\n");

    // We rely on the fact that this can write binary data
    post.Write(postData.mValue.GetBegin(), postData.mValue.Size());

    post.Append("\r\n");
  }

  post.Append("--");
  post.Append(boundary);
  post.Append("--");

  String postString = post.ToString();
  return postString;
}

String WebRequest::GetNewlineSeparatedHeaders()
{
  StringBuilder builder;

  String contentTypeHeader = GetContentTypeHeader();
  builder.Append(contentTypeHeader);

  forRange (StringParam header, mAdditionalRequestHeaders)
  {
    builder.Append(header);
    builder.Append("\r\n");
  }

  String headers = builder.ToString();
  return headers;
}

String WebRequest::GetUserAgent()
{
  // We want to add the version, but we don't know about Engine here.
  // Todo: Move versions into Common.
  // We use this string because it lets servers know we can support many types
  // of responses.
  return "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like "
         "Gecko) Chrome/39.0.2171.71 Safari/537.36 Edge/12.0 Welder";
}

} // namespace Zero
