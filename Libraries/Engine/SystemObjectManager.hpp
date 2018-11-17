///////////////////////////////////////////////////////////////////////////////
///
/// \file SystemObjectManager.hpp
/// Declaration of the SystemObjectManager.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum2(ObjectCleanup, AutoDelete, None);

//System object manager is a simple map of Objects using their typename
//to enforce uniqueness (only one instance per type). It also provides a 
//easy way to clean up all the objects with out needing a large amount 
//of explicit delete or shutdown statements. Global object manager.
class SystemObjectManager : public Object
{
public:
  SystemObjectManager();
  ~SystemObjectManager();

  struct ObjectInstance
  {
    Object* Instance;
    BoundType* InstanceType;
    ObjectCleanup::Enum Cleanup;
  };

  //Get a System or create one if it does not exist.
  template<typename type>
  type* FindOrCreate(cstr name)
  {
    BoundType* metaType = ZilchTypeId(type);

    //Does the object exist?
    HashMap<String, ObjectInstance>::range r =  Objects.Find(metaType->Name);
    if(r.Empty())
    {
      //No create a instance and return it.
      type* newObject = new type();
      Add(newObject, metaType, ObjectCleanup::AutoDelete);
      return newObject;
    }
    
    //return the existing object
    return (type*)r.Front().second.Instance;
  }

  void Add(Object* object, BoundType* meta, ObjectCleanup::Enum cleanup);
  void Add(Object* object, ObjectCleanup::Enum cleanup);

  Object* FindObject(StringParam typeName)
  {
    HashMap<String, ObjectInstance>::range r =  Objects.Find(typeName);
    if(!r.Empty())
      return r.Front().second.Instance;
    return NULL;
  }

  HashMap<String, ObjectInstance> Objects;
};

void StartSystemObjects();
void CleanUpSystemObjects();

namespace Z
{
  extern SystemObjectManager* gSystemObjects;
}

#define CreateSystemObjectFrom(Type, Value)                                      \
    ZilchInitializeType(Type);                                                   \
    Z::gSystemObjects->Add(Value, ZilchTypeId(Type), ObjectCleanup::AutoDelete);

} // namespace Zero
