///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

WebRequest::WebRequest() :
  mOnHeadersReceived(nullptr),
  mOnDataReceived(nullptr),
  mOnComplete(nullptr),
  mUserData(nullptr),
  mCancel(false)
{
}

WebRequest::~WebRequest()
{
}

void WebRequest::Run()
{
}

void WebRequest::Cancel()
{
}

bool WebRequest::IsRunning()
{
  return false;
}

} // namespace Zero
