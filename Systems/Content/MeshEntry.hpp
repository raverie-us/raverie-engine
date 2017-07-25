///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------- Mesh Entry
class MeshEntry : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream)
  {
    SerializeNameDefault(Name, String());
    SerializeNameDefault(mResourceId, (ResourceId)0);
  }

  String Name;
  ResourceId mResourceId;
};

}// namespace Zero