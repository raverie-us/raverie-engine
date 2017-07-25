////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// All Components need to call this in their meta initialization
#define ZeroBindComponent()                                     \
  ZilchBindDefaultConstructor();                                \
  ZilchBindDestructor();

//------------------------------------------------------------------------------------ Meta Resource
// If a Type was created from a Resource, the resource id will be available as a type component.
class MetaResource : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  MetaResource() {}
  MetaResource(Resource* resource);
  
  void SetResource(Resource* resource);

  // The resource this type is defined in.
  ResourceId mResourceId;
  
  // The location this type is defined at.
  CodeLocation mClassLocation;
};

}//namespace Zero
