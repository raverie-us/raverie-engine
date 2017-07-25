/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

// Note: This code is based on Steve Reid's 100% Public Domain Sha1 implementation (thank you Steve!)

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  Sha1Builder::Sha1Builder()
  {
    // SHA1 initialization constants
    this->State[0] = 0x67452301;
    this->State[1] = 0xEFCDAB89;
    this->State[2] = 0x98BADCFE;
    this->State[3] = 0x10325476;
    this->State[4] = 0xC3D2E1F0;
    this->Count[0] = 0;
    this->Count[1] = 0;
  }

  //***************************************************************************
  void Sha1Builder::RunUnitTests()
  {
    const char* inputs[] =
    {
      "abc",
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
      "A million repetitions of 'a'",
      "The quick brown fox jumps over the lazy dog.!@#$%^&**()_+-=[]{}:;',./<>?1234567890 The quick brown fox jumps over the lazy dog.!@#$%^&**()_+-=[]{}:;',./<>?1234567890 The quick brown fox jumps over the lazy dog.!@#$%^&**()_+-=[]{}:;',./<>?1234567890"
    };

    const char* outputs[] =
    {
      "A9993E364706816ABA3E25717850C26C9CD0D89D",
      "84983E441C3BD26EBAAE4AA1F95129E5E54670F1",
      "34AA973CD4C4DAA4F61EEB2BDBAD27316534016F",
      "EB3F49B034A2804826A742754868665D8BCBF4EF"
    };

    const size_t NumTests = sizeof(inputs) / sizeof(const char*);

    for (size_t i = 0; i < NumTests; ++i)
    {
      const char* input = inputs[i];
      const char* expectedOutput = outputs[i];

      Sha1Builder builder;

      // Special case test for test #3 (which is index 2)
      if (i == 2)
      {
        for (size_t i = 0; i < 1000000; ++i)
          builder.Append("a");
      }
      else
      {
        builder.Append(input);
      }
      String result = builder.OutputHashString();

      ErrorIf(result != expectedOutput, "The Sha1Builder returned an incorrect hash");
    }
  }

  //***************************************************************************
  #define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

  /* blk0() and blk() perform the initial expand. */
  /* I got the idea of expanding during the round function from SSLeay */
  /* FIXME: can we do this in an endian-proof way? */
  #ifdef WORDS_BIGENDIAN
  #define blk0(i) block->l[i]
  #else
  #define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
      |(rol(block->l[i],8)&0x00FF00FF))
  #endif
  #define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
      ^block->l[(i+2)&15]^block->l[i&15],1))

  /* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
  #define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
  #define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
  #define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
  #define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
  #define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

  //***************************************************************************
  // Hash a single 512-bit block (this is the core of the algorithm)
  void Sha1BuilderTransform(u32 state[5], const byte buffer[64])
  {
    u32 a, b, c, d, e;
    typedef union
    {
      byte c[64];
      u32 l[16];
    }
    CHAR64LONG16;
    CHAR64LONG16* block;

    block = (CHAR64LONG16*)buffer;

    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;

    /* Wipe variables */
    a = b = c = d = e = 0;
  }
  
  //***************************************************************************
  void Sha1Builder::Append(const byte* data, size_t length)
  {
    size_t i, j;

    j = (this->Count[0] >> 3) & 63;
    if ((this->Count[0] += (u32)(length << 3)) < (length << 3))
      this->Count[1]++;
    this->Count[1] += (u32)(length >> 29);

    if ((j + length) > 63)
    {
      memcpy(&this->Buffer[j], data, (i = 64-j));
      Sha1BuilderTransform(this->State, this->Buffer);
      for (; i + 63 < length; i += 64)
      {
        byte temp[64];
        memcpy(temp, data + i, 64);
        Sha1BuilderTransform(this->State, temp);
      }

      j = 0;
    }
    else
    {
      i = 0;
    }

    memcpy(&this->Buffer[j], &data[i], length - i);
  }
  
  //***************************************************************************
  void Sha1Builder::Append(StringRange data)
  {
    this->Append((byte*)data.Data(), data.SizeInBytes());
  }
  
  //***************************************************************************
  bool Sha1Builder::Append(File& file)
  {
    // If the file isn't valid, early out
    if (file.IsOpen() == false)
      return false;

    // Read all the contents of the file chunk by chunk, running Sha1 on each chunk
    byte buffer[4096] = {0};
    Status status;
    ZilchLoop
    {
      size_t dataRead = file.Read(status, buffer, sizeof(buffer));
      this->Append(buffer, dataRead);

      // if we reached the end or had an error
      if (dataRead == 0 || status.Failed())
        break;
    }

    return true;
  }
  
  //***************************************************************************
  void Sha1Builder::OutputHash(byte* hashOut)
  {
    // Make a temporary copy of the builder
    Sha1Builder copy = *this;

    u32 i;
    byte finalcount[8];

    for (i = 0; i < 8; i++)
    {
      // Endian independent
      finalcount[i] = (unsigned char)((this->Count[(i >= 4 ? 0 : 1)] >> ((3-(i & 3)) * 8) ) & 255);
    }

    this->Append((const byte*)"\200", 1);

    while ((this->Count[0] & 504) != 448)
    {
      this->Append((const byte*)"\0", 1);
    }

    // Should cause a Sha1BuilderTransform()
    this->Append(finalcount, 8);

    for (i = 0; i < Sha1Builder::Sha1ByteSize; i++)
    {
      hashOut[i] = (byte)((this->State[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }

    // Return everything to its original state
    *this = copy;
  }
  
  //***************************************************************************
  void Sha1Builder::OutputHash(Array<byte>& hashOut)
  {
    hashOut.Resize(Sha1Builder::Sha1ByteSize);
    this->OutputHash(hashOut.Data());
  }
  
  //***************************************************************************
  String Sha1Builder::OutputHashString()
  {
    byte hash[Sha1ByteSize] = {0};
    this->OutputHash(hash);

    // Turn the SHA1 into a hex string
    char hexedSha1[Sha1ByteSize * 2 + 1] = {0};
    for (size_t i = 0; i < Sha1ByteSize; ++i)
    {
      ZeroSPrintf(hexedSha1 + 2 * i, 3, "%02X", hash[i]);
    }

    return String(hexedSha1, sizeof(hexedSha1) - 1);
  }
  
  //***************************************************************************
  String Sha1Builder::GetHashString(StringRange data)
  {
    Sha1Builder builder;
    builder.Append(data);
    return builder.OutputHashString();
  }
  
  //***************************************************************************
  String Sha1Builder::GetHashStringFromFile(File& file)
  {
    Sha1Builder builder;
    builder.Append(file);
    return builder.OutputHashString();
  }
  
  //***************************************************************************
  String Sha1Builder::GetHashStringFromFile(Status& status, StringParam fileName)
  {
    // Attempt to open the file
    File file;
    file.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential, Zero::FileShare::Read, &status);
    if (status.Failed())
      return String();

    // Since we opened the file successfully, get the hash from it
    String sha1Hash = Sha1Builder::GetHashStringFromFile(file);
    file.Close();
    return sha1Hash;
  }
}
