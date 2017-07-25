///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyHandle.hpp
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations.
class Object;

//---------------------------------------------------------------- Property Path
DeclareEnum3(PropertyPathType, Component, Property, Index);

// PropertyPath always assumed that a PropertyPathType::Property is at
// the end of the path. This is not the case for some arrays. In the case
// of Array<float>, PropertyPathType::Index will be at the end. This works for now
// because all of our arrays require having a class/struct in them.

/// We used to reference properties with a MetaProperty and a handle to the
/// object instance. The problem that came up was with properties that have
/// their own properties (this is recursive). 
///`
/// PhysicsCar.WheelsCollection.Array[4].CogPath.Required
///
/// Not all properties can be looked up with a MetaHandle and a MetaProperty.
///
/// Example:
/// Take for example the 'Camera' CogPath property on the CameraViewport Component.
/// The full path would be "CameraViewport.Camera.Required" given a Cog as 
/// the path starting point.
/// 
/// The reason for this is because we cannot get a safe handle to the CogPath
/// object with the 'Required' MetaProperty. The root most object we can get
/// a safe handle to is the Cog, so the path must be able to be resolved from
/// there.
class PropertyPath
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Common constructors. For longer paths, use the Add functions.
  PropertyPath(){}
  PropertyPath(Property* prop);
  PropertyPath(cstr prop);
  PropertyPath(StringParam prop);
  PropertyPath(Object* component, Property* prop);
  PropertyPath(Object* component, StringParam propertyName);
  PropertyPath(BoundType* componentType, Property* prop);
  PropertyPath(BoundType* componentType, StringParam propertyName);
  PropertyPath(StringParam componentName, Property* prop);
  PropertyPath(StringParam componentName, StringParam propertyName);

  /// Needed for HashSet.
  bool operator==(const PropertyPath& rhs) const;

  /// CameraViewport.RendererPath.Path
  String GetStringPath() const;
  
  /// Value getter / setter.
  Any GetValue(HandleParam rootInstance) const;
  bool SetValue(HandleParam rootInstance, AnyParam newValue) const;

  /// The path can contain sub-objects, and this function returns
  /// the last instance that Contains the actual MetaProperty.
  Handle GetLeafInstance(HandleParam instance) const;

  /// Returns the meta property from the given leaf instance.
  Property* GetPropertyFromLeaf(HandleParam leafInstance) const;

  /// Returns the meta property from the given root object.
  Property* GetPropertyFromRoot(HandleParam rootInstance) const;

  /// Returns the name of the leaf property name.
  String GetLeafPropertyName() const;

  /// Adds the given component to the path.
  void AddComponentToPath(HandleParam component);
  void AddComponentToPath(BoundType* componentType);
  void AddComponentToPath(StringParam componentName);
  void AddComponentIndexToPath(uint index);

  /// Adds the given property to the path.
  void AddPropertyToPath(Property* prop);
  void AddPropertyToPath(StringParam propertyName);

  void GetInstanceHierarchy(HandleParam rootInstance, Array<Handle>* objects) const;

  /// Returns the leaf instance, and fills out the given object list if given.
  Handle GetLeafInstanceInternal(HandleParam rootInstance,
                                 Array<Handle>* objects = nullptr) const;

  /// Pops the last entry in the property path.
  void PopEntry();

  /// So we can put this into a hashed container.
  size_t Hash() const;

  struct Entry
  {
    Entry() {}
    Entry(StringParam typeName, PropertyPathType::Enum type, uint index = uint(-1)) :
      mName(typeName), mType(type), mIndex(index) { }

    /// TypeName, PropertyName, MemberName
    String mName;
    PropertyPathType::Enum mType;
    /// Used for mType of 'Index'
    uint mIndex;
  };
  Array<Entry> mPath;
};

typedef PropertyPath& PropertyPathRef;
typedef const PropertyPath& PropertyPathParam;

//-------------------------------------------------------------- Property Handle
class PropertPathHandle
{
public:
  /// Constructor.
  PropertPathHandle(HandleParam rootObject, PropertyPathParam path);
  
  /// Value getter / setter.
  Any GetValue();
  bool SetValue(AnyParam newValue);

  Handle mRootObject;
  PropertyPath mPath;
};

}//namespace Zero
