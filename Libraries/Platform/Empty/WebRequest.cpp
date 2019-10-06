// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

WebRequest::WebRequest() :
    mOnHeadersReceived(nullptr),
    mOnDataReceived(nullptr),
    mOnComplete(nullptr),
    mUserData(nullptr)
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

void WebRequest::Initialize()
{
}

void WebRequest::Shutdown()
{
}

} // namespace Zero
