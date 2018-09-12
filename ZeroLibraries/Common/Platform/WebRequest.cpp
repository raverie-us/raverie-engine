///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String WebRequest::GetBoundary()
{
  static const String cBoundary("----------ZeroEngine43476095-a5a0-4190-a9b5-bce2d2de5eef$");
  return cBoundary;
}

String WebRequest::GetContentTypeHeader()
{
  String contentType = BuildString(
    "Content-Type:multipart/form-data; boundary=",
    GetBoundary(),
    "\r\n");

  return contentType;
}

String WebRequest::GetPostDataWithBoundaries()
{
  String boundary = GetBoundary();
  StringBuilder post;

  forRange(WebPostData& postData, mPostData)
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

  forRange(StringParam header, mAdditionalRequestHeaders)
  {
    builder.Append(header);
    builder.Append("\r\n");
  }

  String headers = builder.ToString();
  return headers;
}

} // namespace Zero
