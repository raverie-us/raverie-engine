///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhaseCreator.cpp
/// Implementation of the BroadPhaseCreator, BroadPhaseCreatorType
/// and Library class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
BroadPhaseLibrary* gBroadPhaseLibrary = nullptr;
}//namespace Z

ZilchDefineType(DynamicBroadphasePropertyExtension, builder, type)
{
}

ZilchDefineType(StaticBroadphasePropertyExtension, builder, type)
{
}

BroadPhaseLibrary::BroadPhaseLibrary()
{
  ErrorIf(Z::gBroadPhaseLibrary != nullptr, "Cannot instantiate two Libraries.");
  Z::gBroadPhaseLibrary = this;

  //Register all broad phases
  RegisterBroadPhase(NSquaredBroadPhase,       DynamicBit | StaticBit);
  RegisterBroadPhase(BoundingBoxBroadPhase,    DynamicBit | StaticBit);
  RegisterBroadPhase(BoundingSphereBroadPhase, DynamicBit | StaticBit);
  RegisterBroadPhase(StaticAabbTreeBroadPhase, StaticBit);
  RegisterBroadPhase(SapBroadPhase,            DynamicBit);
  //RegisterBroadPhase(MultiSap, dynamicOnly);
  RegisterBroadPhase(DynamicAabbTreeBroadPhase, DynamicBit | StaticBit);
  RegisterBroadPhase(AvlDynamicAabbTreeBroadPhase, DynamicBit | StaticBit);
}

BroadPhaseLibrary::~BroadPhaseLibrary()
{
  DeleteObjectsInContainer(mBroadPhaseMap);
  Z::gBroadPhaseLibrary = nullptr;
}

void BroadPhaseLibrary::RegisterBroadPhaseCreator(BoundType* type, 
                                                  BroadPhaseCreator* creator, 
                                                  u32 canBeUsedAs)
{
  BroadPhaseMapType::range range = mBroadPhaseMap.Find(type->Name);

  if(!range.Empty())
  {
    ErrorIf(true, "Broad Phase with the name \
                   %s is already registered.", type->Name.c_str());
  }
  else
  {
    mBroadPhaseMap.Insert(type->Name, creator);
    mBroadPhaseTypeMap.Insert(type, creator);
    
    String rawName = RemoveBroadPhaseText(type->Name);
    if(canBeUsedAs & DynamicBit)
      mBroadPhaseNames[BroadPhase::Dynamic].PushBack(rawName);
    if(canBeUsedAs & StaticBit)
      mBroadPhaseNames[BroadPhase::Static].PushBack(rawName);
  }
}

BroadPhaseCreator* BroadPhaseLibrary::GetCreatorBy(PolymorphicNode& broadPhaseNode)
{
  if(broadPhaseNode.RuntimeType==nullptr)
  {
    //Find the component's creator using the string typename
    BroadPhaseMapType::range range =  mBroadPhaseMap.Find(broadPhaseNode.TypeName);
    ErrorIf(range.Empty(), "Could not find broad phase creator with name '%s'."
      "Bad file? Bad broad phase name? BroadPhase not registered?", String(broadPhaseNode.TypeName).c_str() ); 

    if(!range.Empty())
      return range.Front().second;
    else
      return nullptr;
  }
  else
  {
    //Find the component's creator using the TypeId
    BroadPhaseIdMapType::range range =  mBroadPhaseTypeMap.Find(broadPhaseNode.RuntimeType);
    ErrorIf(range.Empty(), "Could not find broad phase creator for '%s'. Bad file?", broadPhaseNode.RuntimeType->Name);
    if(!range.Empty())
      return range.Front().second;
    else
      return nullptr;
  }
}

BroadPhaseCreator* BroadPhaseLibrary::GetCreatorBy(StringParam name)
{
  BroadPhaseMapType::range range =  mBroadPhaseMap.Find(name);
  if(range.Empty())
    return nullptr;
  return range.Front().second;
}

IBroadPhase* BroadPhaseLibrary::CreateBroadPhase(StringParam name)
{
  //if the name of the broadphase is missing the "BroadPhase" text at the end then Append it
  String trueName = name;
  StringRange range = trueName.FindFirstOf("BroadPhase");
  if(range.Empty())
    trueName = BuildString(trueName, "BroadPhase");

  BroadPhaseCreator* creator = GetCreatorBy(trueName);
  if(creator == nullptr)
    return nullptr;
  return creator->Create();
}

void BroadPhaseLibrary::EnumerateNames(Array<String>& names)
{
  BroadPhaseMapType::range r = mBroadPhaseMap.All();
  while(!r.Empty())
  {
    String name = RemoveBroadPhaseText(r.Front().first);
    names.PushBack(name);
    r.PopFront();
  }
}

void BroadPhaseLibrary::EnumerateNamesOfType(BroadPhase::Type type, Array<String>& names)
{
  names.Assign(mBroadPhaseNames[type].All());
}

String BroadPhaseLibrary::RemoveBroadPhaseText(StringParam name)
{
  StringRange r = name.All();
  while(!r.Empty())
  {
    String curr(r);
    if(curr == "BroadPhase")
      return String(name.Data(), r.Data());
    r.PopFront();
  }

  return String();
}

}//namespace Zero
