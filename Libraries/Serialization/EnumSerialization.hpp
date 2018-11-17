///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
#define SerializeEnumName(enumName, variable) \
  stream.EnumField(enumName::EnumName, #variable, (uint&)variable, ZilchTypeId(enumName::Enum))

#define SerializeEnumCustomName(enumName, variableName, variable) \
  stream.EnumField(enumName::EnumName, variableName, (uint&)variable, ZilchTypeId(enumName::Enum))

#define SerializeEnumNameDefault(enumName, variable, defaultValue) \
  if(!stream.EnumField(enumName::EnumName, #variable, (uint&)variable, ZilchTypeId(enumName::Enum))) \
  variable = defaultValue;

// Bit Field Serialization 
template <typename BitFieldType>
void SerializeBits(Serializer& stream, BitFieldType& field, const cstr enumNames[], uint mask = 0, uint defaults = uint(-1))
{
  if(stream.GetType() == SerializerType::Text)
  {
    if(stream.GetMode() == SerializerMode::Saving)
    {
      for(uint i = 0; enumNames[i] != NULL; ++i)
      {
        uint currBit = 1 << i;

        //Continue if we're ignoring this bit
        if((currBit & mask) > 0)
          continue;

        bool state = field.IsSet(currBit);
        stream.SerializeField(enumNames[i], state);
      }
    }
    else
    {
      for(uint i = 0; enumNames[i] != NULL; ++i)
      {
        uint currBit = 1 << i;

        //Continue if we're ignoring this bit
        if((currBit & mask) > 0)
          continue;

        bool state = false;
        bool defaultState = (currBit & defaults) != 0;
        stream.SerializeFieldDefault(enumNames[i], state, defaultState);
        field.SetState(currBit,state);
      }
    }
  }
  else //Could this be done in a better way?
  {
    //Serialize the number in binary ignoring the masked bits
    if(stream.GetMode() == SerializerMode::Loading)
    {
      //We want to keep the state of everything in the mask and only override the new values.
      //To do this clear the bits we are saving then "or" in the serialized data.
      u32 maskedField = field.Field & mask;
      field.U32Field = maskedField;
      stream.SerializeField("BitField", maskedField);
      field.U32Field |= maskedField;
    }
    else
    {
      u32 maskedField = field.Field & ~mask;
      stream.SerializeField("BitField", maskedField);
    }
  }
}

}//namespace Zero
