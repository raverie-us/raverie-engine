// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieThreadLocal ErrorContextStack* ActiveErrorContextStack = nullptr;

ErrorContext::ErrorContext()
{
  // Check to see if the stack is allocated
  if (ActiveErrorContextStack == nullptr)
    ActiveErrorContextStack = new ErrorContextStack();

  // Push onto stack
  ActiveErrorContextStack->PushBack() = this;
}

ErrorContext::~ErrorContext()
{
  // auto pop from stack
  ActiveErrorContextStack->PopBack();
};

String ErrorContextObject::GetDescription()
{
  // If a ContextObject is set get a
  // display string for that object
  String objectDesc;

  if (ContextObject)
  {
    BoundType* metaType = RaverieVirtualTypeId(ContextObject);
    if (MetaDisplay* display = metaType->HasInherited<MetaDisplay>())
      objectDesc = display->GetDebugText(Handle(ContextObject));
    else
      objectDesc = metaType->ToStringFunction(metaType, (const byte*)ContextObject);
  }

  // Return the display string combined with
  return String::Format("%s - %s", Message, objectDesc.c_str());
}

void DoNotifyErrorWithContext(StringParam message, NotifyException::Enum notifyException)
{
  // Notify first
  DoNotify("Error", message, "Warning", NotifyType::Error, notifyException);

  // If the notification is set to ignore, then don't report the error context
  if (Z::gNotifyCallbackStack.Back() == IgnoreDoNotify)
    return;

  // Print every error context level
  forRange (ErrorContext* context, ActiveErrorContextStack->All())
  {
    String contextMessage = context->GetDescription();
    ZPrint("\t%s\n", contextMessage.c_str());
  }
}

void CleanUpErrorContext()
{
  if (ActiveErrorContextStack)
    SafeDelete(ActiveErrorContextStack);
}

} // namespace Raverie
