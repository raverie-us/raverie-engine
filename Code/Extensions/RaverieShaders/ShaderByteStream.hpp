// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// A writer object base class to write to a shader stream.
class ShaderStreamWriter
{
public:
  virtual ~ShaderStreamWriter(){};
  virtual void WriteWord(uint32 word) = 0;

  void Write(uint32 word);
  void Write(uint16 highOrder, uint16 lowOrder);
  void Write(uint8 a, uint8 b, uint8 c, uint8 d);
  void WriteInstruction(uint16 size, uint16 instruction);
  void WriteInstruction(uint16 size, uint16 instruction, uint32 data0);
  void WriteInstruction(uint16 size, uint16 instruction, uint32 data0, uint32 data1);
  void WriteInstruction(uint16 size, uint16 instruction, uint32 data0, uint32 data1, uint32 data2);
  void WriteInstruction(uint16 instruction, Array<uint32>& args);

  void Write(StringParam text);
  size_t GetPaddedByteCount(StringParam text);
};

/// Shader data that results from a pipeline pass.
class ShaderByteStream
{
public:
  /// Load an array of words into this byte stream.
  void Load(Array<uint32>& words);
  /// Load a string into this byte stream.
  void Load(StringParam str);

  /// Load memory into this byte stream (copies data).
  void Load(const char* source, size_t sizeInBytes);
  /// Load memory into this byte stream (copies data).
  void Load(const byte* source, size_t sizeInBytes);
  /// Load memory from an array of words (copies data).
  /// Simple helper to avoid copying word->byte conversions everywhere.
  void LoadWords(const uint32* data, size_t wordCount);

  /// Save to an array of words
  void SaveTo(Array<uint32>& words);

  /// Convert data to a string. Assumes the underlying data was actually a
  /// string (only valid depending on the backend type, such as glsl).
  String ToString();
  byte* Data();
  size_t ByteCount() const;
  size_t WordCount() const;

  Array<byte> mData;
};

/// A writer for a shader byte stream.
class ShaderByteStreamWriter : public ShaderStreamWriter
{
public:
  ShaderByteStreamWriter();
  ShaderByteStreamWriter(ShaderByteStream* stream);
  ~ShaderByteStreamWriter();

  void WriteWord(uint32 word) override;

  byte* Data();
  size_t ByteCount() const;
  size_t WordCount() const;

  ShaderByteStream* mByteStream;
  bool mStreamIsOwned;
};

} // namespace Raverie
