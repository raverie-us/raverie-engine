///////////////////////////////////////////////////////////////////////////////
///
/// \file ZeroContainers.hpp
/// Declaration of the serialization polices for standard types.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Serialization
{

template<>
struct Trait<String>
{
  enum{Type=StructureType::Value};
  static inline cstr TypeName(){ return "string"; }
};

template<typename type>
struct Trait< Array<type> >
{
  enum{Type=StructureType::Array};
  static inline cstr TypeName(){ return "Array"; }
};

template<typename type>
struct Trait< PodArray<type> >
{
  enum{Type=StructureType::Array};
  static inline cstr TypeName(){ return "Array"; }
};

template<typename keytype, typename valuetype>
struct Trait< HashMap<keytype, valuetype>  >
{
  enum{Type=StructureType::Array};
  static inline cstr TypeName(){ return "Map"; }
};

template<typename type>
struct Trait< HashSet<type> >
{
  enum{Type=StructureType::Array};
  static inline cstr TypeName(){ return "Array"; }
};

template<typename keytype, typename valuetype>
struct Trait< UnsortedMap<keytype, valuetype>  >
{
  enum{Type=StructureType::Array};
  static inline cstr TypeName(){ return "Map"; }
};

template<typename keytype, typename valuetype>
struct Trait< ArrayMap<keytype, valuetype> >
{
  enum{Type=StructureType::Array};
  static inline cstr TypeName() { return "Map"; };
};

template<>
struct Policy<String>
{
  static inline bool Serialize(Serializer& serializer, cstr fieldName, String& stringValue)
  {
    if(serializer.GetMode() == SerializerMode::Saving)
    {
      StringRange stringRange = stringValue.All();
      serializer.StringField("string", fieldName, stringRange);
      return true;
    }
    else
    {
      StringRange stringRange;
      if(serializer.StringField("string", fieldName, stringRange))
      {
        stringValue = stringRange;
        return true;
      }
    }
    return false;
  }
};

template<typename type>
struct SerializeFunctor
{
  SerializeFunctor(Serializer& s)
    :stream(s) {}
  void operator()(type& object){ stream.SerializeValue(object); }
  Serializer& stream;
};

template<typename first, typename second>
struct Policy< Pair<first, second> >
{
  typedef Pair<first, second> type;
  static inline bool Serialize(Serializer& serializer, cstr fieldName, Pair<first,second>& pair)
  {
    serializer.Start(Trait<type>::TypeName(), fieldName, Serialization::Trait<type>::Type);
    serializer.SerializeField("key", pair.first);
    serializer.SerializeField("value", pair.second);
    serializer.End(Trait<type>::TypeName(), Serialization::Trait<type>::Type);
    return true;
  }
};

template<typename containerType>
void SaveSequence(Serializer& serializer, containerType& container)
{
  uint containerSize = (uint)container.Size();
  serializer.ArraySize(containerSize);
  ForEach(container.All(), SerializeFunctor<typename containerType::value_type>(serializer));
}

template<typename containerType>
void LoadSequence(Serializer& serializer, containerType& container)
{
  uint containerSize = 0;
  serializer.ArraySize(containerSize);
  container.Resize(containerSize);
  ForEach(container.All(), SerializeFunctor<typename containerType::value_type>(serializer));
}

template<typename containerType>
void InsertSequence(Serializer& serializer, containerType& container)
{
  container.Clear();
  uint containerSize = 0;
  serializer.ArraySize(containerSize);
  for(uint i=0;i<containerSize;++i)
  {
    typename containerType::value_type tempValue;
    serializer.SerializeValue(tempValue);
    container.Insert(tempValue);
  }
}

template<typename type>
static inline bool SerializeSequence(Serializer& stream, cstr fieldName, type& sequence)
{
  bool started = stream.Start(Trait<type>::TypeName(), fieldName,  Serialization::Trait<type>::Type);
  if(started)
  {
    if(stream.GetMode() == SerializerMode::Saving)
    {
      SaveSequence(stream, sequence);
    }
    else
    {
      LoadSequence(stream, sequence);
    }
    stream.End(Trait<type>::TypeName(), Serialization::Trait<type>::Type);
  }
  return started;
}

template<typename type>
inline bool SerializeSequenceInsert(Serializer& stream, cstr fieldName, type& sequence)
{
  bool started = stream.Start(Trait<type>::TypeName(), fieldName, Serialization::Trait<type>::Type);
  if(started)
  {
    if(stream.GetMode() == SerializerMode::Saving)
    {
      SaveSequence(stream, sequence);
    }
    else
    {
      InsertSequence(stream, sequence);
    }
    stream.End(Trait<type>::TypeName(), Serialization::Trait<type>::Type);
  }
  return started;
}

template<typename type>
struct Policy< Array<type>  >
{
  typedef Array<type> containertype;

  static inline bool Serialize(Serializer& serializer, cstr fieldName, containertype& container)
  {
    return SerializeSequence(serializer, fieldName, container);
  }
};

template<typename type>
struct Policy< PodArray<type>  >
{
  typedef PodArray<type> containertype;
  static inline bool Serialize(Serializer& serializer, cstr fieldName, containertype& container)
  {
    return SerializeSequence(serializer, fieldName, container);
  }
};

template<typename keytype, typename valuetype>
struct Policy< HashMap<keytype, valuetype>  >
{
  typedef HashMap<keytype, valuetype>  containertype;
  static inline bool Serialize(Serializer& serializer, cstr fieldName, containertype& container)
  {
    return SerializeSequenceInsert(serializer, fieldName, container);
  }
};

template<typename keytype, typename valuetype>
struct Policy< UnsortedMap<keytype, valuetype>  >
{
  typedef UnsortedMap<keytype, valuetype>  containertype;
  static inline bool Serialize(Serializer& serializer, cstr fieldName, containertype& container)
  {
    return SerializeSequence(serializer, fieldName, container);
  }
};

template<typename type>
struct Policy< HashSet<type>  >
{
  typedef HashSet<type>  containertype;
  static inline bool Serialize(Serializer& serializer, cstr fieldName, containertype& container)
  {
    return SerializeSequenceInsert(serializer, fieldName, container);
  }
};

template<typename keytype, typename valuetype>
struct Policy< ArrayMap<keytype, valuetype> >
{
  typedef ArrayMap<keytype, valuetype> containertype;
  static inline bool Serialize(Serializer& serializer, cstr fieldName, containertype& container)
  {
    return SerializeSequence(serializer, fieldName, container);
  }
};

}//namespace Serialization
}//namespace Zero
