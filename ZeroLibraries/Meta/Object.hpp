///////////////////////////////////////////////////////////////////////////////
///
/// \file Object.hpp
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class EventReceiver;
class EventDispatcher;
class Actions;

//----------------------------------------------------------------------- Object
/// Core Object class every engine and game object inherits from this class.
class Object : public IZilchObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  OverloadedNew();

  Object(){};
  virtual ~Object();

  // Events
  virtual EventDispatcher* GetDispatcherObject(){return NULL;}
  virtual EventReceiver* GetReceiverObject(){return NULL;}
  EventDispatcher* GetDispatcher(){return GetDispatcherObject();}

  // Actions
  virtual Actions* GetActions(){return NULL;}

  // Serialization
  virtual void Serialize(Serializer& serializer){}

  // Used for objects that require custom memory clean up like scripts
  virtual void Delete() { delete this; }

  // Returns whether or not the property existed.
  bool SetProperty(StringParam propertyName, AnyParam val);
  Any GetProperty(StringParam propertyName);
};

typedef void* ObjPtr;

} // namespace Zero
