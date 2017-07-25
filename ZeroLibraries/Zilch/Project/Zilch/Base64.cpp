/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

// Note: This code is based on libb64's public domain code

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  const int CharactersPerLine = 72;
  namespace Base64EncodeStep
  {
    enum Enum
    {
      A,
      B,
      C
    };
  }

  //***************************************************************************
  String Base64Encoder::Encode(const byte* data, size_t length)
  {
    Base64Encoder encoder;
    size_t computedLength = ComputeSize(length);

    // The data that we're encoding as we go along
    // Eventually this should probably be refactored to be a ByteBuffer (similar to other builders)
    Zero::StringNode* node = String::AllocateNode(computedLength);
    byte* encodedData = (byte*)node->Data;
    size_t written = encoder.EncodeData(data, length, encodedData);
    encoder.EncodeEnd(encodedData + written);

    return String(node);
  }

  //***************************************************************************
  Base64Encoder::Base64Encoder()
  {
    this->Step = Base64EncodeStep::A;
    this->Result = 0;
    this->StepCount = 0;
  }

  //***************************************************************************
  size_t Base64Encoder::ComputeSize(size_t length)
  {
    return (size_t)(std::ceil(length / 3.0f)) * 4;
  }
  
  //***************************************************************************
  char Base64Encoder::EncodeValue(char value)
  {
    static const char* Encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if (value > 63)
      return '=';

    return Encoding[value];
  }
  
  //***************************************************************************
  size_t Base64Encoder::EncodeData(const byte* data, size_t length, byte* output)
  {
    const char* plainchar = (const char*)data;
    const char* plaintextend = plainchar + length;
    char* code_out = (char*)output;
    char* codechar = code_out;
    char result;
    char fragment;

    result = this->Result;

    switch (this->Step)
    {
      ZilchLoop
      {
    case Base64EncodeStep::A:
        if (plainchar == plaintextend)
        {
          this->Result = result;
          this->Step = Base64EncodeStep::A;
          return codechar - code_out;
        }
        fragment = *plainchar++;
        result = (fragment & 0x0fc) >> 2;
        *codechar++ = EncodeValue(result);
        result = (fragment & 0x003) << 4;

      case Base64EncodeStep::B:
        if (plainchar == plaintextend)
        {
          this->Result = result;
          this->Step = Base64EncodeStep::B;
          return codechar - code_out;
        }
        fragment = *plainchar++;
        result |= (fragment & 0x0f0) >> 4;
        *codechar++ = EncodeValue(result);
        result = (fragment & 0x00f) << 2;

      case Base64EncodeStep::C:
        if (plainchar == plaintextend)
        {
          this->Result = result;
          this->Step = Base64EncodeStep::C;
          return codechar - code_out;
        }
        fragment = *plainchar++;
        result |= (fragment & 0x0c0) >> 6;
        *codechar++ = EncodeValue(result);
        result  = (fragment & 0x03f) >> 0;
        *codechar++ = EncodeValue(result);

        ++(this->StepCount);
        if (this->StepCount == CharactersPerLine / 4)
        {
          *codechar++ = '\n';
          this->StepCount = 0;
        }
      }
    }
    
    // We should never reach this case...
    return codechar - code_out;
  }
  
  //***************************************************************************
  size_t Base64Encoder::EncodeEnd(byte* output)
  {
    char* codechar = (char*)output;

    switch (this->Step)
    {
      case Base64EncodeStep::B:
        *codechar++ = EncodeValue(this->Result);
        *codechar++ = '=';
        *codechar++ = '=';
        break;
      case Base64EncodeStep::C:
        *codechar++ = EncodeValue(this->Result);
        *codechar++ = '=';
        break;
      case Base64EncodeStep::A:
        break;
    }

    return codechar - (char*)output;
  }
}
