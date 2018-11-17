///////////////////////////////////////////////////////////////////////////////
///
/// \file Notification.cpp
/// Implementation of the basic notification classes.
///
/// Authors: Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
  Array<NotificationCallback> gNotifyCallbackStack(ZeroInit, DefaultDoNotify);
}

namespace Events
{
  DefineEvent(Notify);
}

ZilchDefineType(NotifyEvent, builder, type)
{
}

void DoNotify(StringParam title, StringParam message, StringParam icon, NotifyType::Enum type, NotifyException::Enum expections)
{
  Z::gNotifyCallbackStack.Back()(title, message, icon, type, expections);
}

void DoNotifyErrorNoAssert(StringParam title, StringParam message)
{
  DoNotify(title, message, "Error", NotifyType::Error);
}

void DoNotifyError(StringParam title, StringParam message)
{
  WarnIf(true, "%s", message.c_str());
  DoNotify(title, message, "Error", NotifyType::Error);
}

void DoNotifyWarning(StringParam title, StringParam message)
{
  DoNotify(title, message, "Warning", NotifyType::Warning);
}

void DoNotifyException(StringParam title, StringParam message)
{
  DoNotify(title, message, "Warning", NotifyType::Error, NotifyException::Script);
}

void DoNotifyExceptionAssert(StringParam title, StringParam message)
{
  WarnIf(true, "%s", message.c_str());
  DoNotify(title, message, "Warning", NotifyType::Error, NotifyException::Script);
}

void DefaultDoNotify(StringParam title, StringParam message, StringParam icon, NotifyType::Enum type,  NotifyException::Enum exceptions)
{
  String finalMessage = String::Format("%s: %s", title.c_str(), message.c_str());
  Error(finalMessage.c_str());
}

void IgnoreDoNotify(StringParam title, StringParam message, StringParam icon, NotifyType::Enum type,  NotifyException::Enum exceptions)
{
  // We do nothing here, this is used to suppress notifications (such as when we use previews)
}

TemporaryDoNotifyOverride::TemporaryDoNotifyOverride(NotificationCallback callback)
{
  mCallback = callback;

  if(callback != nullptr)
    Z::gNotifyCallbackStack.PushBack(callback);
}

TemporaryDoNotifyOverride::~TemporaryDoNotifyOverride()
{
  if(mCallback != nullptr)
    Z::gNotifyCallbackStack.PopBack();
}

void DoNotifyStatus(Status& status)
{
  if(status.Failed())
  {
    String message = String::Format("%s\n", status.Message.c_str());
    Error("%s", message.c_str());
    DoNotify("Error", message, "Error", NotifyType::General);
  }
}

}//namespace Zero
