///////////////////////////////////////////////////////////////////////////////
///
/// \file ResourcePropertyOperations.cpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//*****************************************************************************
void GetResourcesFromProperties(HandleParam object,
                                HashSet<Resource*>& resources)
{
  //METAREFACTOR We should optimize all these cases by making a special range class that does not allocate
  // Originally the range class allocated to store values safely for Zilch...
  // (does it actually matter for Zilch, since all types are safe?)
  BoundType* objectType = object.StoredType;
  forRange(Property* metaProperty, objectType->GetProperties())
  {
    // We're only looking for dependencies that are serialized. Any runtime references don't matter.
    if(metaProperty->HasAttribute(PropertyAttribute) == false && metaProperty->HasAttribute(PropertyAttributes::cSerialized) == false)
      continue;

    Type* propType = metaProperty->PropertyType;
    
    // Can't do anything if it isn't a resource
    if(propType->IsA(ZilchTypeId(Resource)) == false)
      continue;

    // Grab the current value
    Any value = metaProperty->GetValue(object);

    // Add the resource
    Resource* resource = value.Get<Resource*>(GetOptions::AssertOnNull);
    if(resource)
      resources.Insert(resource);
  }

  // Walk the composition
  if(MetaComposition* composition = objectType->has(MetaComposition))
  {
    uint componentCount = composition->GetComponentCount(object);
    for(uint i = 0; i < componentCount; ++i)
    {
      // Add all resources from the components properties
      Handle component = composition->GetComponentAt(object, i);
      GetResourcesFromProperties(component, resources);

      // The component itself may be a resource, so check that as well
      MetaResource* fromResource = component.StoredType->has(MetaResource);
      if(fromResource != nullptr && fromResource->mResourceId != 0)
      {
        // Look up the resource and add it if it exists
        Resource* resource = Z::gResources->GetResource(fromResource->mResourceId);
        if(resource)
          resources.Insert(resource);
      }

      // Consider walking the dependencies of each component
    }
  }
}

}//namespace Zero
