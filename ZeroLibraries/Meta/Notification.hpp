///////////////////////////////////////////////////////////////////////////////
///
/// \file EngineEvents.hpp
/// Declaration of the notify event and functions.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(Notify);
}

DeclareEnum3(NotifyType, Error, Warning, General);
DeclareEnum2(NotifyException, None, Script);

//----------------------------------------------------------------- Notify Event
class NotifyEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  NotifyType::Enum Type;
  String Name;
  String Message;
  String Icon;
};

#define DoNotifyOnce(title, message, icon)  \
{                                           \
  static bool __Once = true;                \
  if(__Once)                                \
{                                           \
  DoNotify(title, message, icon);           \
  __Once = false;                           \
}                                           \
}

#define DoNotifyCounter(title, message, icon, counter)  \
{                                                       \
  static int __Counter = counter;                       \
  --__Counter;                                          \
  if(__Counter == 0)                                    \
{                                                       \
  DoNotify(title, message, icon);                       \
  __Counter = counter;                                  \
}                                                       \
}

#define DoNotifyTimer(title, message, icon, seconds)  \
{                                                     \
  static Timer __timer;                               \
  __timer.Update();                                   \
  if(__timer.Time() >= double(seconds))               \
{                                                     \
  DoNotify(title, message, icon);                     \
  __timer.Reset();                                    \
}                                                     \
}

void DoNotify(StringParam title, StringParam message, StringParam icon, 
  NotifyType::Enum type = NotifyType::General,
  NotifyException::Enum expections = NotifyException::None);

void DoNotifyWarning(StringParam title, StringParam message);
void DoNotifyError(StringParam title, StringParam message);
void DoNotifyErrorNoAssert(StringParam title, StringParam message);
void DoNotifyException(StringParam title, StringParam message);
void DoNotifyExceptionAssert(StringParam title, StringParam message);
void DoNotifyStatus(Status& status);

void DefaultDoNotify(StringParam title, StringParam message, StringParam icon, NotifyType::Enum type, NotifyException::Enum exceptions);
void IgnoreDoNotify(StringParam title, StringParam message, StringParam icon, NotifyType::Enum type, NotifyException::Enum exceptions);

typedef void (*NotificationCallback)(StringParam title, StringParam message,
  StringParam icon, NotifyType::Enum type, NotifyException::Enum exceptions);

// Allows us to temporarily override DoNotify (will get cleaned up upon destruction)
// If a 'nullptr' callback is passed in, this will do nothing (it won't push or pop a callback)
// Use 'IgnoreDoNotify' to cause notifications to stop
class TemporaryDoNotifyOverride
{
public:
  NotificationCallback mCallback;
  TemporaryDoNotifyOverride(NotificationCallback callback);
  ~TemporaryDoNotifyOverride();
};

namespace Z
{
  extern Array<NotificationCallback> gNotifyCallbackStack;
}

}
