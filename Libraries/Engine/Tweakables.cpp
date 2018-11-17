///////////////////////////////////////////////////////////////////////////////
///
/// \file Tweakables.cpp
/// Provides an easy way to bind constants to be tweaked in the editor.
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// All values that were proxied are stored here
HashMap<void*, Any> sProxyValues;

//------------------------------------------------------------ Global Tweakables
namespace Z
{
  Tweakables* gTweakables = nullptr;
}//namespace Z

//******************************************************************************
void SerializeTweakableNode(HandleParam object, Serializer& serializer)
{
  MetaSerializeObject(object, serializer);
}

//******************************************************************************
void TweakableSetter(Call& call, ExceptionReport& report)
{
  Property* property = call.GetFunction()->OwningProperty;

  byte* parameter = call.GetParameterUnchecked(0);

  if (property->UserData == nullptr)
  {
    Any any(parameter, property->PropertyType);
    sProxyValues[property] = any;
    return;
  }

  property->PropertyType->GenericCopyConstruct((byte*)property->UserData, parameter);

  if (Tweakables::sModifiedCallback)
    (*Tweakables::sModifiedCallback)();
}

//******************************************************************************
void TweakableGetter(Call& call, ExceptionReport& report)
{
  Property* property = call.GetFunction()->OwningProperty;

  if (property->UserData == nullptr)
  {
    Any val = sProxyValues.FindValue(property, Any());
    call.Set(Call::Return, val);
  }

  Type* returnType = property->PropertyType;
  byte* returnLocation = call.GetReturnUnchecked();
  call.DisableReturnChecks();

  returnType->GenericCopyConstruct(returnLocation, (const byte*)property->UserData);
}

//---------------------------------------------------------------- TweakableNode

//******************************************************************************
TweakableNode::TweakableNode(StringParam typeName)
{
  Name = typeName;
  Meta = nullptr;
}

//******************************************************************************
TweakableNode::~TweakableNode()
{
  DeleteObjectsInContainer(Children.mArray);
  DeleteObjectsInContainer(mProperties);
}


//******************************************************************************
BoundType* TweakableNode::ZilchGetDerivedType() const
{
  return Meta;
}

//******************************************************************************
void TweakableNode::Serialize(Serializer& stream)
{
  if(stream.GetMode() == SerializerMode::Saving)
  {
    MetaSerializeObject(this, stream);
  }
  else
  {
    DataTreeLoader& loader = *(DataTreeLoader*)(&stream);

    forRange(TweakableProperty* property, mProperties.Values())
      property->Serialize(loader);

    PolymorphicNode node;
    while(loader.GetPolymorphic(node))
    {
      // Look up the tweakable node
      String nodeName = node.TypeName;

      TweakableNode* childNode = Children.FindValue(nodeName, nullptr);

      if(childNode == nullptr)
      {
        childNode = new TweakableNode(nodeName);
        Children.Insert(nodeName, childNode);
      }

      childNode->Serialize(stream);

      loader.EndPolymorphic();
    }
  }
}

//------------------------------------------------------------------- Tweakables
Tweakables::TweakableModifiedCallback Tweakables::sModifiedCallback = NULL;
String Tweakables::mFileName;

ZilchDefineType(Tweakables, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

//******************************************************************************
Tweakables::Tweakables()
  : TweakableNode("Tweakables")
{
}

//******************************************************************************
void Tweakables::Initialize()
{
  // Initialize it if it doesn't exist
  if(Z::gTweakables == NULL)
    Z::gTweakables = new Tweakables();
}

//******************************************************************************
void Tweakables::Save()
{
  // For the meta refactor, we're making this not save at all
  if (true)
    return;

  // Don't allow saving to tweakables unless the user has dev config
  DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig);
  if(devConfig == nullptr)
    return;

  // Get the file location
  String file = GetFileLocation();
  // Can we write to the tweakables? (Installed versions can't because they're in program files)
  if(FileWritable(file) == false)
    return;

  // Open the file
  TextSaver saver;
  Status status;
  saver.Open(status, file.c_str());

  ErrorIf(status.Failed(), "Failed to save Tweakables file.");

  // Start saving from the root object
  Z::gTweakables->Serialize(saver);
}

//******************************************************************************
void Tweakables::Load(StringParam fileName)
{
  mFileName = fileName;

  // Get the file location
  String file = GetFileLocation();

  // Open the file for loading
  DataTreeLoader loader;
  Status status;
  loader.OpenFile(status, file);

  // Load the object. It's valid to not exist
  if(!status.Failed())
  {
    PolymorphicNode rootNode;
    loader.GetPolymorphic(rootNode);
    Z::gTweakables->Serialize(loader);
    loader.EndPolymorphic();
  }
}

//******************************************************************************
String Tweakables::GetFileLocation()
{
  // Look up the data folder in the configuration file
  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  String directory = mainConfig->DataDirectory;

  // Combine the directory and file
  return FilePath::CombineWithExtension(directory, mFileName, ".data");
}

//-------------------------------------------------------- TweakablesComposition
//******************************************************************************
TweakablesComposition::TweakablesComposition() :
  MetaComposition(nullptr)
{
  mSupportsComponentRemoval = false;
}

//******************************************************************************
uint TweakablesComposition::GetComponentCount(HandleParam owner)
{
  TweakableNode* node = owner.Get<TweakableNode*>(GetOptions::AssertOnNull);
  return node->Children.Size();
}

Handle TweakablesComposition::GetComponentAt(HandleParam owner, uint index)
{
  // It's an array map, so we can just directly index it
  TweakableNode* node = owner.Get<TweakableNode*>(GetOptions::AssertOnNull);
  TweakableNode* child = node->Children[index].second;
  return child;
}

//******************************************************************************
Handle TweakablesComposition::GetComponent(HandleParam owner, BoundType* componentType)
{
  // We have to search each node for the correct type Id
  // We could store them in another map to make this faster, however
  // it's only used for loading once in the editor and there won't be that many,
  // so it's not all that important
  TweakableNode* node = owner.Get<TweakableNode*>(GetOptions::AssertOnNull);
  forRange(TweakableNode* child, node->Children.AllValues())
  {
    if(child->Meta == componentType)
      return child;
  }

  return Handle();
}

//******************************************************************************
BoundType* TweakablesComposition::GetComponentMeta(StringParam name)
{
  return MetaDatabase::GetInstance()->FindType(name);
}

}//namespace Zero
