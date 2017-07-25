///////////////////////////////////////////////////////////////////////////////
///
/// \file Tweakables.hpp
/// Provides an easy way to bind constants to be tweaked in the editor.
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{


//----------------------------------------------------------- TweakableNode Type
class TweakableProperty
{
public:
  virtual ~TweakableProperty() {}
  virtual void Serialize(DataTreeLoader& loader) = 0;
};

template <typename PropertyType>
class TweakablePropertyType : public TweakableProperty
{
public:
  TweakablePropertyType(PropertyType* value, StringParam name) : mValue(value), mName(name) {}

  void Serialize(DataTreeLoader& loader) override
  {
    loader.SerializeField(mName.c_str(), *mValue);
  }

  PropertyType* mValue;
  String mName;
};

//---------------------------------------------------------------- TweakableNode
class TweakableNode : public EventObject
{
public:

  /// Constructor.
  TweakableNode(StringParam typeName);
  virtual ~TweakableNode();

  /// Object Interface
  BoundType* ZilchGetDerivedType() const override;
  void Serialize(Serializer& stream) override;

  /// All child nodes.
  ArrayMultiMap<String, TweakableNode*> Children;

  /// We need our own meta for each as we're binding different
  /// properties to each node.
  /// Note that this meta is null when we first create all tweakable nodes
  /// and is filled out after when we build the library via Tweakables
  String Name;
  BoundType* Meta;

  HashMap<String, TweakableProperty*> mProperties;
};

//------------------------------------------------------------------- Tweakables
class Tweakables : public TweakableNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  /// Constructor.
  Tweakables();

  /// Searches the path for the correct parent and binds the
  /// tweakable as a MetaProperty.
  template <typename PropertyType>
  void RegisterTweakable(PropertyType* value, PropertyType& defaultValue,
                         cstr name, cstr location);

  /// Initializes the Tweakables system.
  static void Initialize();

  /// Saves the tweakables to the Data folder.
  static void Save();

  /// Loads the tweakables to the Data folder.
  static void Load(StringParam fileName);

  /// Returns the location where we're saving the file (data folder).
  static String GetFileLocation();

  /// The name of the file.
  static String mFileName;

  /// A callback to let the editor know to update all Ui.
  typedef void (*TweakableModifiedCallback)();
  static TweakableModifiedCallback sModifiedCallback;

  //LibraryRef mLibrary;
};

//-------------------------------------------------------- TweakablesComposition
class TweakablesComposition : public MetaComposition
{
public:
  TweakablesComposition();

  /// MetaComposition Interface.
  uint GetComponentCount(HandleParam owner) override;
  Handle GetComponentAt(HandleParam owner, uint index) override;
  Handle GetComponent(HandleParam owner, BoundType* componentType) override;
  BoundType* GetComponentMeta(StringParam name);
};

//----------------------------------------------------------------- TweakableVar
template<typename type>
class TweakableVar
{
public:
  TweakableVar(type defaultValue, cstr name, cstr location);
  operator type();

  type mValue;
};

//----------------------------------------------------------------------- Macros
#define Tweakable(type, name, defaultValue, location) TweakableVar<type> name(defaultValue, #name, location);
#define DeclareTweakable(type, name) extern TweakableVar<type> name;

//------------------------------------------------------------ Global Tweakables
namespace Z
{
  extern Tweakables* gTweakables;
}//namespace Z

#include "Tweakables.inl"

}//namespace Zero
