///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhaseCreator.hpp
/// Declaration of the BroadPhaseCreator, BroadPhaseCreatorType
/// and Library class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct PolymorphicNode;

///Used to create broad phases in data driven method.
class BroadPhaseCreator
{
public:
  virtual IBroadPhase* Create()=0;
  virtual ~BroadPhaseCreator(){}
};

///Builder class used to make concrete BroadPhase creators.
template <typename type>
class BroadPhaseCreatorType : public BroadPhaseCreator
{
public:
  virtual IBroadPhase* Create(){return new type;}
};

class BroadPhaseLibrary
{
public:
  BroadPhaseLibrary();
  ~BroadPhaseLibrary();
  void RegisterBroadPhaseCreator(BoundType* type, BroadPhaseCreator* creator, 
                                 u32 canBeUsedAs);

  BroadPhaseCreator* GetCreatorBy(PolymorphicNode& broadPhaseNode);
  BroadPhaseCreator* GetCreatorBy(StringParam name);
  IBroadPhase* CreateBroadPhase(StringParam name);
  void EnumerateNames(Array<String>& names);
  void EnumerateNamesOfType(BroadPhase::Type type, Array<String>& names);

private:
  String RemoveBroadPhaseText(StringParam name);

  ///Map of Strings to BroadPhaseCreators used for data driven broad phase
  ///from text files.
  typedef HashMap<String, BroadPhaseCreator*> BroadPhaseMapType;
  BroadPhaseMapType mBroadPhaseMap;

  ///Map of type to BroadPhaseCreators used for data driven
  ///loading of broad phases.
  typedef HashMap<BoundType*, BroadPhaseCreator*> BroadPhaseIdMapType;
  BroadPhaseIdMapType mBroadPhaseTypeMap;

  Array<String> mBroadPhaseNames[BroadPhase::Size];
};

namespace Z
{
  extern BroadPhaseLibrary* gBroadPhaseLibrary;
}//namespace Z

#define RegisterBroadPhase(type, canBeUsedAs) \
  Z::gBroadPhaseLibrary->RegisterBroadPhaseCreator(ZilchTypeId(type), new BroadPhaseCreatorType<type>(), canBeUsedAs);

/// A property editor for enumerating broadphase types
/// (BroadPhaseType is used to distinguish between Static and Dynamic Broadphases)
template <uint BroadphaseType>
class BroadphasePropertyExtension : public EditorIndexedStringArray
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  static void EnumerateBroadphases(HandleParam instance, Property* property, Array<String>& strings)
  {
    Z::gBroadPhaseLibrary->EnumerateNamesOfType(BroadphaseType, strings);
  }

  BroadphasePropertyExtension()
    : EditorIndexedStringArray(&BroadphasePropertyExtension::EnumerateBroadphases)
  {
  }
};

typedef BroadphasePropertyExtension<BroadPhase::Dynamic> DynamicBroadphasePropertyExtension;
typedef BroadphasePropertyExtension<BroadPhase::Static> StaticBroadphasePropertyExtension;

}//namespace Zero
