///////////////////////////////////////////////////////////////////////////////
///
/// \file Serialization.hpp
/// Declaration of the Serializer interface.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------------- Data Attributes
namespace SerializationAttributes
{
DeclareStringConstant(Id);
DeclareStringConstant(InheritId);
DeclareStringConstant(ChildOrderOverride);
DeclareStringConstant(LocallyAdded);
DeclareStringConstant(LocallyRemoved);
}

struct DataAttribute
{
  DataAttribute() {}
  DataAttribute(StringParam name, StringParam value);

  String mName;
  String mValue;
};

typedef Array<DataAttribute> DataAttributes;

//---------------------------------------------------------------- Data Version
DeclareEnum2(DataVersion,
  // DataVersion 0 is when there was no data versions
  Legacy,
  // Data versioning was added at this release
  Current
);

//-------------------------------------------------------------------- Constants
const cstr cPolymorphicSerializationError = "Polymorphic serialization not "
  "supported on this serializer. When using polymorphic serialization "
  "you must separate saving and loading. When saving use Start Polymorphic / "
  "End Polymorphic. When reading you must use Get Polymorphic, Create the object "
  "with the type specified in the polymorphic node, serialize the object and then "
  "call End Polymorphic.";

//------------------------------------------------------------- Polymorphic Node
DeclareBitField4(PolymorphicFlags,
        // This node was inherited from another data file 
        // (such as an Archetype)
        Inherited,
        // This means the node had a '-' in front of it, whatever that
        // means in the context of what is being serialized. This was added
        // as an optimization for loading Archetypes with local
        // modifications in lieu of full data tree patching.
        Subtractive,
        ChildOrderOverride,
        // Was this node patched in any way
        Patched);

//------------------------------------------------------------- Polymorphic Node
struct PolymorphicNode
{
  PolymorphicNode() : mAttributes(nullptr), UniqueNodeId(cInvalidUniqueNodeId) {}

  StringRange TypeName;
  StringRange Name;
  BoundType* RuntimeType;
  uint ChildCount;
  uint Version;
  uint Offset;
  /// A way to uniquely identify polymorphic children. It was initially added
  /// to identify locally modified child Cogs in an Archetype.
  Guid UniqueNodeId;
  BitField<PolymorphicFlags::Enum> Flags;
  StringRange mInheritId;

  /// If the serializer provides attributes for polymorphic nodes, they will
  /// be accessible here. It will be null otherwise.
  DataAttributes* mAttributes;

  static const Guid cInvalidUniqueNodeId;
};

// Structure Type for text serializers
DeclareEnum4(StructureType,
  // Basic Value Type like integers or strings
  Value,
  // Object Type that contain name value fields
  Object,
  // Array Type that Contains simple values
  Array,
  BasicArray
  );

// Structure Type for text serializers
DeclareEnum2(BasicArrayType,
  Float,
  Integer
  );

// SerializerMode Saving or Loading
DeclareEnum2(SerializerMode,
  //Data is being saved to this stream
  Saving,
  //Data is being loaded from this stream
  Loading
  );

// SerializerType for what features are supported
DeclareEnum3(SerializerType,
  //Binary serializes support raw write/reads
  //and use integer type Ids.
  Binary,
  //Text serializers support string typeIds
  //and test for value presence.
  Text,
  //Generic does not support type ids.
  //Only Start, End, and  Fundamental.
  Generic
  );

// Since the serializer doesn't have access to Meta, we can't do an 'IsA' 
// check/dynamic cast, so use this instead
DeclareEnum7(SerializerClass, BinaryLoader, BinarySaver, TextLoader, 
             TextSaver, DataTreeLoader, DefaultSerializer, SerializerBuilder);

// Flags used to change how the polymorphic node is saved out
DeclareBitField2(PolymorphicSaveFlags,
  // A polymorphic node can be set to LocallyAdded when saving out a data tree patch.
  // The polymorphic node will be added to the inherited data tree.
  LocallyAdded,
  // If set, the child order of this node will be used to override the order
  // of the tree that's being patched.
  ChildOrderOverride
  );

// Used for data tree patching
typedef void(*PatchCallback)(cstr fieldName, void* clientData);

//------------------------------------------------------------- Polymorphic Info
/// Information used to open a polymorphic 
struct PolymorphicInfo
{
  PolymorphicInfo();

  /// The typename that will be saved out.
  cstr mTypeName;

  /// Used to uniquely reference a polymorphic child.
  Guid mUniqueNodeId;

  /// Field name if it has a field.
  cstr mFieldName;

  /// Used for data patching.
  String mInheritanceId;

  BitField<PolymorphicSaveFlags::Enum> mFlags;

  /// Generally used in place of the type name for binary serialization.
  BoundType* mRuntimeType;
};

//------------------------------------------------------------------- Serializer
/// Base Serializer. Concrete serializers (Text, Binary, Data)
/// implement this interface. This separates the serialization process
/// (what data needs to be read) from the serialization format (Text, Binary)
class Serializer
{
public:
  // Typedefs
  typedef uint StructType;
  typedef uint ArrayType;

  /// Constructor / destructor.
  Serializer();
  virtual ~Serializer();

  virtual void Close() {};


  void* GetSerializationContext();
  void SetSerializationContext(void* context);

  /// Mode of serialization Saving or Loading
  SerializerMode::Enum GetMode();

  /// Type of serialization
  SerializerType::Enum GetType();

  virtual SerializerClass::Enum GetClass() = 0;

  virtual bool Start(cstr typeName, cstr fieldName, StructType structType) = 0;
  bool Start(BoundType* type, cstr fieldName, StructType structType);
  virtual void End(cstr typeName, StructType structType) = 0;
  void End(BoundType* type, StructType structType);


  /// Polymorphic Serialization Interface.
  void StartPolymorphic(cstr typeName);
  void StartPolymorphic(cstr typeName, PolymorphicSaveFlags::Enum flags);
  void StartPolymorphic(BoundType* runtimeType);
  void StartPolymorphicInheritence(cstr typeName, cstr dataInheritanceId);
  void StartPolymorphicInheritence(cstr typeName, cstr dataInheritanceId,
                                   PolymorphicSaveFlags::Enum flags);
  virtual void StartPolymorphicInternal(const PolymorphicInfo& info);
  virtual void EndPolymorphic() = 0;
  virtual bool GetPolymorphic(PolymorphicNode& node) = 0;

  /// Tells data tree patching to remove the node of the given type name.
  virtual void AddSubtractivePolymorphicNode(cstr typeName,
                          Guid nodeId = PolymorphicNode::cInvalidUniqueNodeId) {}
  void AddSubtractivePolymorphicNode(BoundType* boundType,
                                     Guid nodeId = PolymorphicNode::cInvalidUniqueNodeId);
  virtual void ArraySize(uint& arraySize) = 0;

#define FUNDAMENTAL(type) virtual bool FundamentalField(cstr fieldName, type& value) = 0;
#include "FundamentalTypes.hpp"
#undef FUNDAMENTAL

  virtual bool SimpleField(cstr typeName, cstr fieldName, StringRange& stringRange);
  virtual bool EnumField(cstr enumTypeName, cstr fieldName, uint& enumValue, BoundType* type) = 0;
  virtual bool StringField(cstr typeName, cstr fieldName, StringRange& stringRange) = 0;
  virtual bool ArrayField(cstr typeName, cstr fieldName, byte* data,
    ArrayType arrayType, uint numberOfElements, uint sizeOftype) = 0;

  virtual DataBlock ExtractAsDataBlock();

  // Describe  current location stream the stream.
  // used when there is serialization errors to provide debug information
  virtual String DebugLocation();

  // Serialization Helpers
  template<typename type>
  void SerializeValue(type& instance);

  template<typename type>
  void SerializeField(cstr fieldName, type& instance);

  template<typename type>
  void SerializeFieldDefault(cstr fieldName, type& instance, const type& defaultValue, cstr oldFieldName = nullptr);

  template<typename type>
  void SerializeFieldRename(cstr oldFieldName, type& instance);

  template<typename type>
  void SerializePolymorphic(type& instance);

  template<typename type>
  void SerializePolymorphic(const PolymorphicInfo& info, type& instance);

  /// If we're in patching mode while loading, we only want to serialize
  /// values if they existed in the stream (don't set defaults for properties
  /// that weren't serialized).
  bool mPatching;

  /// Signals that a property was patched.
  PatchCallback mPatchCallback;

  /// Client data that goes with the patch callback.
  void* mPatchClientData;

protected:
  void* mSerializationContext;
  SerializerMode::Enum mMode;
  SerializerType::Enum mSerializerType;
};

#define SerializeName(variable) \
  stream.SerializeField(#variable, variable)

#define SerializeNameDefault(variable, defaultValue) \
  stream.SerializeFieldDefault(#variable, variable, defaultValue)

#define SerializeRename(variable, oldFieldName) \
  stream.SerializeFieldRename(oldFieldName, variable)

#define SerializeByteBufferBlock(name, byteBufferBlock) \
  if(stream.GetMode() == SerializerMode::Loading)      \
  {                                                     \
    String temp;                                        \
    stream.SerializeFieldDefault(name, temp, String()); \
    Zero::DecodeBinary(byteBufferBlock, temp);          \
  }                                                     \
  else                                                  \
  {                                                     \
    String temp;                                        \
    Zero::EncodeBinary(byteBufferBlock, temp);          \
    stream.SerializeFieldDefault(name, temp, String()); \
  }

}

#include "SerializationTraits.hpp"
#include "EnumSerialization.hpp"

namespace Zero
{

template<typename type>
void Serializer::SerializeValue(type& instance)
{
  bool success =  Serialization::Policy<type>::Serialize(*this, NULL, instance);
  ErrorIf(!success, "Failed to serialize value at %s.", this->DebugLocation().c_str());
}

template<typename type>
void Serializer::SerializeField(cstr fieldName, type& instance)
{
  bool success = Serialization::Policy<type>::Serialize(*this, fieldName, instance);
  ErrorIf(!success, "Failed to serialize value. %s at %s", fieldName, this->DebugLocation().c_str());
}

template<typename type>
void Serializer::SerializeFieldDefault(cstr fieldName, type& instance,
                                       const type& defaultValue, cstr oldFieldName)
{
  bool success = Serialization::Policy<type>::Serialize(*this, fieldName, instance);

  // Try the old name if the first didn't succeed
  if(!success && oldFieldName)
    success = Serialization::Policy<type>::Serialize(*this, oldFieldName, instance);

  if(mPatching)
  {
    // Signal that there was a successful patch
    if(success && mPatchCallback)
      mPatchCallback(fieldName, mPatchClientData);
  }
  // We don't want to set the default value if we're patching
  else if(!success)
    instance = defaultValue;
}

template<typename type>
void Serializer::SerializeFieldRename(cstr oldFieldName, type& instance)
{
  if (mSerializerType == SerializerType::Text && mMode == SerializerMode::Loading)
  {
    type fieldValue;
    if (FundamentalField(oldFieldName, fieldValue))
      instance = fieldValue;
  }
}

template<typename type>
void Serializer::SerializePolymorphic(type& instance)
{
  BoundType* type = ZilchVirtualTypeId(&instance);
  StartPolymorphic(type);
  instance.Serialize(*this);
  EndPolymorphic();
}

template<typename type>
void Serializer::SerializePolymorphic(const PolymorphicInfo& info, type& instance)
{
  StartPolymorphicInternal(info);
  instance.Serialize(*this);
  EndPolymorphic();
}

void EncodeBinary(ByteBufferBlock& buffer, String& encodedOut);
bool DecodeBinary(ByteBufferBlock& buffer, const String& encoded);

}//namespace Zero
