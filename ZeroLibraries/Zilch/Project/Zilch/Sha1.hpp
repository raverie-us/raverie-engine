/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

// Note: This code is based on Steve Reid's 100% Public Domain Sha1 implementation (thank you Steve!)

#pragma once
#ifndef ZILCH_SHA1_HPP
#define ZILCH_SHA1_HPP

namespace Zilch
{
  class ZeroShared Sha1Builder
  {
  public:
    static const size_t Sha1ByteSize = 20;

    Sha1Builder();

    // Updates the Sha1 with new data
    void Append(const byte* data, size_t length);

    // Updates the Sha1 with string data
    void Append(StringRange data);

    // Appends all the contents of a file to the hash (read chunk by chunk)
    // Returns true if it succeeded, or false if the file was not open or invalid
    bool Append(File& file);

    // Outputs the hash to an array of bytes (does not modify our builder)
    // The byte array must be at least 'Sha1ByteSize'
    void OutputHash(byte* hashOut);

    // Resizes the array and outputs the hash to it (does not modify our builder)
    void OutputHash(Array<byte>& hashOut);

    // Outputs the hash to a hex string (does not modify our builder)
    String OutputHashString();

    // Gets a Sha1 hash from string data
    static String GetHashString(StringRange data);

    // Gets a Sha1 hash from string data
    static String GetHashStringFromFile(File& file);

    // Gets a Sha1 hash from string from a file
    static String GetHashStringFromFile(Status& status, StringParam fileName);
    
    // Tests the implementation of Sha1
    static void RunUnitTests();
  private:

    u32 State[5];
    u32 Count[2];
    byte Buffer[64];
  };

}
#endif
