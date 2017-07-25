///////////////////////////////////////////////////////////////////////////////
///
/// \file Event.cpp
/// Implementation of the Event class and support.
///
/// Authors: Chris Peters.
/// Copyright 2010-2012, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(SendableEvent, builder, type)
{
  ZeroBindDocumented();
}

SendableEvent::SendableEvent() :
  Connection(NULL)
{
}

void SendableEvent::Serialize(Serializer& stream)
{
  SerializeName(EventId);
}

void SendableEvent::Delete()
{
  delete this;
}

SendableEvent* SendableEvent::Load(Serializer& stream)
{
  // Deserialize the name of the object we wish to create
  PolymorphicNode node;
  stream.GetPolymorphic(node);

  // Store the meta data that we get from polymorphic serialization
  BoundType* boundType = nullptr;

  // If we have a type name...
  if (node.TypeName.Empty() == false)
  {
    // Lookup the meta by type name
    boundType = MetaDatabase::GetInstance()->FindType(node.TypeName);
  }
  else
  {
    // Otherwise
    boundType = node.RuntimeType;
  }

  // Make sure meta inherits from sendable event
  // Remove for now
  //ErrorIf(meta->IsNotA(MetaTypeOf(SendableEvent)), "The event must bind/inherit from the base class SendableEvent");

  // If we actually found the meta data...
  if (boundType != nullptr)
  {
    //METAREFACTOR This may not work since we're using the calling state for everything
    // Create the object with no flags, then serialize it's data into the loader
    HandleOf<SendableEvent> event = ZilchAllocate(SendableEvent, boundType);

    // Make sure we got a packet object
    if (!event.IsNull())
    {
      // Deserialize the object
      event->Serialize(stream);

      // Return the deserialized event
      return event;
    }
    else
    {
      // Throw a warning since we can't actually create that event type
      Error("Unable to create event of type '%s', was its meta initialized with InitializeMetaOfTypeWithCreator?", String(node.TypeName).c_str());
    }
  }
  else
  {
    // Throw a warning since we got data that we don't know what it is
    Error("Cannot find meta for object type '%s', was its meta initialized with InitializeMetaOfTypeWithCreator?", String(node.TypeName).c_str());
  }

  // Otherwise, if we got here, return nullptr since we did not properly deserialize the event
  return nullptr;
}

void SendableEvent::Save(SendableEvent* event, Serializer& stream)
{
  // Get the meta class for the event
  BoundType* eventType = ZilchVirtualTypeId(event);

  // Make sure that the meta class exists
  ErrorIf(eventType == nullptr, "The event that's being serialized has no meta class, which means that InitializeMetaOfType was never called");

  // Save the type name so we can read it on the other end
  stream.StartPolymorphic(eventType);

  // Serialize the packet into the saver
  event->Serialize(stream);

  // End the object
  stream.EndPolymorphic();
}

} // namespace Zero
