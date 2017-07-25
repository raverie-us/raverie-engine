///////////////////////////////////////////////////////////////////////////////
///
/// \file ObjectSaver.hpp
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class ObjectState;

//struct ObjectSaverData
//{
//  Object* mPropertyPathParent;
//  PropertyPath mCurrentPath;
//  bool mPatching;
//  InheritIdContext::Enum mContext;
//};

//------------------------------------------------------------------------------------- Object Saver
/// This will be in charge of properly saving out data tree patches based
/// on the given object and their local modifications.
class ObjectSaver : public TextSaver
{
public:
  /// Constructor.
  ObjectSaver();

  /// Saves the given object to the opened stream. If the given object inherits from other data
  /// (as defined by the MetaDataInheritance on MetaType), it will be saved out as a patch
  /// of the inherited data tree based on the objects local modifications.

  /// 
  /// e.g. saving a Cog inside a level
  void SaveInstance(Object* object);

  /// 
  /// e.g. uploading to Archetype or saving to a Material file.
  void SaveDefinition(Object* object);

private:
  void SaveObject(Object* object, Object* propertyPathParent, PropertyPath& path, bool patching,
                  InheritIdContext::Enum context);

  /// Save out the entire object.
  void SaveFullObject(Object* object);
  
  /// Save out only properties / child objects that are locally modified.
  void SaveModifications(Object* object, Object* propertyPathParent, PropertyPath& path,
                         InheritIdContext::Enum context);

  void SaveProperties(Object* object, Object* propertyPathParent, PropertyPath& path,
                      bool onlyModifiedProperties);
  void SaveChildren(Object* object, Object* propertyPathParent, PropertyPath& path,
                    bool onlyModifiedChildren);

  void BuildPolymorphicInfo(PolymorphicInfo& info, Object* object, InheritIdContext::Enum context,
                            bool patching);

  //bool IsPatching();

  /// When we save an object starting in the middle of a hierarchy (e.g. uploading to Archetype on
  /// a Cog that's underneath another Archetype), there are some attributes we don't want to save
  Object* mSerializeStart;

  /// How many inheritance nodes deep we are.
  //uint mInheritanceDepth;

  /// All objects that have locally modified children - optimization for checking if child objects
  /// are modified as we recurse saving children
  //HashSet<Object*> mChildModified;
};

}//namespace Zero
