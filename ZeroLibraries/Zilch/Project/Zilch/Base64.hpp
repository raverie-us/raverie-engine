/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

// Note: This code is based on libb64's public domain code

#pragma once
#ifndef ZILCH_BASE64_HPP
#define ZILCH_BASE64_HPP

namespace Zilch
{
  class Base64Encoder
  {
  public:

    // Base64 encoding will always be longer than the original binary length (by a factor of 4/3)
    static size_t ComputeSize(size_t length);

    // Encodes data and returns the base64 string (human readable)
    static String Encode(const byte* data, size_t length);

  private:
    Base64Encoder();
    char EncodeValue(char value);
    size_t EncodeData(const byte* data, size_t length, byte* output);
    size_t EncodeEnd(byte* output);

    // Internals to the base64 encoding algorithm
    int Step;
    char Result;
    int StepCount;
  };
}
#endif
