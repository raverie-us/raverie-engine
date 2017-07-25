///////////////////////////////////////////////////////////////////////////////
///
/// \file Tweakables.inl
/// Provides an easy way to bind constants to be tweaked in the editor.
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------- TweakableVar
//******************************************************************************
template<typename type>
TweakableVar<type>::TweakableVar(type defaultValue, cstr name, cstr location)
{
  // Initialize it to the default value
  mValue = defaultValue;

  // Make sure the root object is initialized
  Tweakables::Initialize();

  // Register the tweakable
  Z::gTweakables->RegisterTweakable(&mValue, defaultValue, name, location);
}

//******************************************************************************
template <typename PropertyType>
void Tweakables::RegisterTweakable(PropertyType* tweakable, PropertyType& defaultValue,
                                   cstr name, cstr location)
{
  // Find the parent of the tweakable (start at the root)
  TweakableNode* parent = this;

  // Walk each object
  forRange(StringRange nodeName, StringTokenRange(location, '/'))
  {
    // Add an underscore to the end of the name to avoid conflicts
    String typeName = BuildString(nodeName, "_");

    // If it doesn't exist, we need to create a new node
    TweakableNode* node = parent->Children.FindValue(typeName, NULL);

    if(node == NULL)
    {
      // Create and add the node
      node = new TweakableNode(typeName);
      parent->Children.Insert(typeName, node);
    }

    // Continue on to the next object
    parent = node;
  }

  // At this point, we should have our parent node
  parent->mProperties.InsertOrError(name, new TweakablePropertyType<PropertyType>(tweakable, name));

  //// Create the new property with our custom getters and setters
  //MetaProperty* customProperty = new MetaProperty(name, typeId, 
  //                                          &TweakableGetter, &TweakableSetter);
  //// We want it to be serialized
  //customProperty->Flags.SetFlag(PropertyFlags::Serialized);
  //customProperty->UserData = tweakable;
  //customProperty->DefaultValue = defaultValue;
  //
  //// Add the meta property
  //parent->Meta->BindMetaProperty(customProperty, NULL);
}

//******************************************************************************
template<typename type>
TweakableVar<type>::operator type()
{
  return mValue;
}
