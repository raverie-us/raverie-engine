// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
static const String cHdrSignature("#?RADIANCE");

static bool FindNewline(Stream* stream, char* newline)
{
  while (!stream->IsEof())
  {
    byte character = stream->ReadByte();
    if (character == '\r')
    {
      newline[0] = '\r';
      if (!stream->IsEof() && stream->PeekByte() == '\n')
      {
        // Since we just peeked, we need to advance the stream
        stream->ReadByte();
        newline[1] = '\n';
      }
      return true;
    }
    else if (character == '\n')
    {
      newline[0] = '\n';
      return true;
    }
  }

  return false;
}

DeclareEnum3(ReadLineStatus, ReadTextLine, ReadEmptyLine, Eof);

static ReadLineStatus::Enum ReadLine(Stream* stream, cstr newline)
{
  ReadLineStatus::Enum result = ReadLineStatus::ReadEmptyLine;

  // Find newline location
  while (!stream->IsEof())
  {
    if (stream->ReadByte() == newline[0])
    {
      if (newline[1] == 0)
        break;
      else if (!stream->IsEof() && stream->PeekByte() == newline[1])
      {
        // Since we just peeked, we need to advance the stream
        stream->ReadByte();
        break;
      }
    }
    else
    {
      result = ReadLineStatus::ReadTextLine;
    }
  }

  // No data or didn't find newline
  if (stream->IsEof())
    result = ReadLineStatus::Eof;

  // The stream position should now be past the newline
  return result;
}

class HdrHeaderGrammar
{
public:
  static HdrHeaderGrammar& GetInstance();

  HdrHeaderGrammar();

  GrammarSet<Character> mTokenGrammar;
  GrammarSet<Token> mParserGrammar;

  GrammarRule<Character>* mTokenStart;
  GrammarRule<Token>* mStartRule;
};

HdrHeaderGrammar& HdrHeaderGrammar::GetInstance()
{
  static HdrHeaderGrammar sInstance;
  return sInstance;
}

HdrHeaderGrammar::HdrHeaderGrammar()
{
  GrammarRule<Character>& tokenStart = mTokenGrammar["TokenStart"];
  GrammarRule<Character>& comment = mTokenGrammar["Comment"];
  GrammarRule<Character>& whitespace = mTokenGrammar["Whitespace"];
  GrammarRule<Character>& token = mTokenGrammar["Token"];
  GrammarRule<Character>& assignment = mTokenGrammar["Assignment"];
  GrammarRule<Character>& sign = mTokenGrammar["Sign"];

  tokenStart |= comment | whitespace | token | assignment | sign;
  comment |= T("#") << *T("^\r\n");
  whitespace |= *T(" \t\r\n");
  token |= T("a-zA-Z_0-9") << *T("a-zA-Z_0-9");
  assignment |= T("=") << *T("^\r\n");
  sign |= T("+-");

  mTokenGrammar.AddIgnore(whitespace);

  GrammarRule<Token>& start = mParserGrammar["Start"];
  GrammarRule<Token>& statement = mParserGrammar["Statement"];
  GrammarRule<Token>& variable = mParserGrammar["Variable"];
  GrammarRule<Token>& dimensions = mParserGrammar["Dimensions"];

  start |= P("FirstComment", P(comment)) << *statement << dimensions;
  statement |= P(comment) | variable;
  variable |= P("Name", P(token)) << P("Value", P(assignment));
  dimensions |= P("YSign", P(sign)) << P("YName", P(token)) << P("YDim", P(token)) << P("XSign", P(sign))
                                    << P("XName", P(token)) << P("XDim", P(token));

  mTokenStart = &tokenStart;
  mStartRule = &start;
}

class HdrHeaderParser
{
public:
  static bool Parse(Status& status, StringParam header, uint& width, uint& height);

  // Only grabbing values on EndRule
  void StartRule(GrammarRule<Token>* rule)
  {
  }
  void EndRule(ParseNodeInfo<Token>* info);
  void TokenParsed(ParseNodeInfo<Token>* info)
  {
  }
  void StartParsing()
  {
  }
  void EndParsing()
  {
  }

  String mFileIdentifier;
  String mFormat;
  String mXSpec;
  String mYSpec;
  String mWidth;
  String mHeight;
};

bool HdrHeaderParser::Parse(Status& status, StringParam header, uint& width, uint& height)
{
  HdrHeaderGrammar& grammar = HdrHeaderGrammar::GetInstance();

  TokenStream<> tokenStream;
  tokenStream.mRange = TokenRange<>(grammar.mTokenGrammar, *grammar.mTokenStart, header);

  HdrHeaderParser hdrHeaderParser;

  RecursiveDescentParser<Token, TokenStream<>, HdrHeaderParser> parser;
  parser.mParseHandler = &hdrHeaderParser;
  parser.mStartRule = grammar.mStartRule;
  parser.mStream = &tokenStream;

  parser.Parse();

  if (hdrHeaderParser.mFileIdentifier != "#?RADIANCE")
  {
    status.SetFailed("File is not an HDR image, file did not start with '#?RADIANCE'");
    return false;
  }

  if (hdrHeaderParser.mFormat != "32-bit_rle_rgbe")
  {
    status.SetFailed("Unsupported format, must be 32-bit_rle_rgbe");
    return false;
  }

  if (hdrHeaderParser.mXSpec != "+X" || hdrHeaderParser.mYSpec != "-Y")
  {
    status.SetFailed("Unsupported layout, must be '-Y <dim> +X <dim>'");
    return false;
  }

  if (hdrHeaderParser.mWidth.Empty() || hdrHeaderParser.mHeight.Empty())
  {
    status.SetFailed("Failed to identify image dimensions");
    return false;
  }

  ToValue(hdrHeaderParser.mWidth, width);
  ToValue(hdrHeaderParser.mHeight, height);

  return true;
}

void HdrHeaderParser::EndRule(ParseNodeInfo<Token>* info)
{
  GrammarRule<Token>* rule = info->mRule;
  String ruleName = rule->mName;

  if (ruleName == "Start")
  {
    Token commentToken = info->GetFirstCapturedToken("FirstComment");
    mFileIdentifier = commentToken.mString;
  }
  else if (ruleName == "Variable")
  {
    Token nameToken = info->GetFirstCapturedToken("Name");
    Token valueToken = info->GetFirstCapturedToken("Value");

    if (nameToken.mString == "FORMAT")
    {
      String value = valueToken.mString;
      // Strip '='
      value = value.SubString(++value.Begin(), value.End());
      // Strip any whitespace
      value = value.TrimStart();
      mFormat = value;
    }
  }
  else if (ruleName == "Dimensions")
  {
    Token xSignToken = info->GetFirstCapturedToken("XSign");
    Token xNameToken = info->GetFirstCapturedToken("XName");
    Token xDimToken = info->GetFirstCapturedToken("XDim");
    Token ySignToken = info->GetFirstCapturedToken("YSign");
    Token yNameToken = info->GetFirstCapturedToken("YName");
    Token yDimToken = info->GetFirstCapturedToken("YDim");

    mXSpec = BuildString(xSignToken.mString, xNameToken.mString);
    mYSpec = BuildString(ySignToken.mString, yNameToken.mString);

    mWidth = xDimToken.mString;
    mHeight = yDimToken.mString;
  }
}

void ParseHdrHeader(Status& status, Stream* stream, uint& width, uint& height)
{
  // Determine newline type
  char newline[2] = {0};
  if (FindNewline(stream, newline) == false)
  {
    status.SetFailed("No header found");
    return;
  }

  // Find empty line for end of header
  while (!stream->IsEof())
  {
    ReadLineStatus::Enum line = ReadLine(stream, newline);
    if (line == ReadLineStatus::Eof)
    {
      status.SetFailed("No end of header found (empty line)");
      return;
    }

    if (line == ReadLineStatus::ReadEmptyLine)
      break;
  }

  // Read past resolution line
  if (ReadLine(stream, newline) != ReadLineStatus::ReadTextLine)
  {
    status.SetFailed("No resolution line after header");
    return;
  }

  // Seek back to the beginning and create a String out of the header data
  size_t headerSize = (size_t)stream->Tell();
  Raverie::StringNode* node = Raverie::String::AllocateNode(headerSize);
  stream->Seek(0);
  stream->Read((byte*)node->Data, headerSize);
  String header(node);

  HdrHeaderParser::Parse(status, header, width, height);
}

// Used to make sure no data reads go out of bounds
#define ValidateRead(data, endData, count)                                                                             \
  if (data + (count) > endData)                                                                                        \
    return false;

bool DecodeHdrScanline(byte*& imageData, const byte* endData, uint imageWidth, byte* scanline)
{
  uint index = 0;
  ValidateRead(imageData, endData, index + 4);

  byte byte0 = imageData[index++];
  byte byte1 = imageData[index++];
  byte byte2 = imageData[index++];
  byte byte3 = imageData[index++];

  uint scanWidth = (uint)byte2 << 8 | byte3;

  // If first two bytes are of value 2 and next two are the high and low bytes
  // of the scanline width then scanline is adaptive run length encoded
  if (byte0 == 2 && byte1 == 2 && scanWidth == imageWidth)
  {
    // All four channels (R, G, B, E) are stored separately in this encoding
    for (uint channel = 0; channel < 4; ++channel)
    {
      uint pixel = 0;
      while (pixel < scanWidth)
      {
        ValidateRead(imageData, endData, index + 2);

        // A value greater than 128 means a run of the same value (and actual
        // count is without the high bit), less than 128 means a run of
        // different values, equal to 128 means nothing and is invalid
        uint count = imageData[index++];
        if (count > 128)
        {
          count -= 128;
          byte value = imageData[index++];

          // Stride is every 4 bytes since channels are separated
          for (uint run = 0; run < count; ++run)
            scanline[4 * pixel++ + channel] = value;
        }
        else
        {
          ValidateRead(imageData, endData, index + count);

          // Stride is every 4 bytes since channels are separated
          for (uint run = 0; run < count; ++run)
            scanline[4 * pixel++ + channel] = imageData[index++];
        }
      }
    }
  }
  // Scanline data is flat (no compression)
  else
  {
    uint scanlineSize = imageWidth * 4;
    ValidateRead(imageData, endData, scanlineSize);
    memcpy(scanline, imageData, scanlineSize);
    index = scanlineSize;
  }

  imageData += index;
  return true;
}

bool IsHdr(Stream* stream)
{
  size_t signatureSize = cHdrSignature.SizeInBytes();
  byte* buffer = (byte*)alloca(signatureSize);
  size_t amountRead = stream->Read(buffer, signatureSize);
  stream->Seek(0);
  return amountRead == signatureSize && memcmp(buffer, cHdrSignature.Data(), signatureSize) == 0;
}

bool ReadHdrInfo(Stream* stream, ImageInfo& info)
{
  info.Format = TextureFormat::RGB32f;

  Status status;
  ParseHdrHeader(status, stream, info.Width, info.Height);
  stream->Seek(0);
  return status.Succeeded();
}

bool IsHdrLoadFormat(TextureFormat::Enum format)
{
  return format == TextureFormat::None || IsHdrSaveFormat(format);
}

bool IsHdrSaveFormat(TextureFormat::Enum format)
{
  return format == TextureFormat::RGB32f;
}

void LoadHdr(Status& status,
             Stream* stream,
             byte** output,
             uint* width,
             uint* height,
             TextureFormat::Enum* format,
             TextureFormat::Enum requireFormat)
{
  if (!IsHdrLoadFormat(requireFormat))
  {
    status.SetFailed("Hdr only supports the format RGB32f");
    return;
  }

  // Note that ParseHdrHeader advances the stream.
  uint imageWidth, imageHeight;
  ParseHdrHeader(status, stream, imageWidth, imageHeight);
  if (status.Failed())
    return;

  // At this point, the stream will be just after the header (where the data
  // starts). Subtract header size
  size_t size = (size_t)(stream->Size() - stream->Tell());
  ByteBufferBlock block;
  stream->ReadMemoryBlock(status, block, size);

  if (status.Failed())
    return;

  byte* imageData = block.GetBegin();
  const byte* endData = imageData + size;

  // Allocate full size of final image
  float* outputImage = (float*)zAllocate(sizeof(float) * imageWidth * imageHeight * 3);

  if (!outputImage)
  {
    status.SetFailed("Failed to allocate memory for the Hdr output image");
    return;
  }

  // Scratch buffer for decoding a single scanline
  byte* scanline = (byte*)zAllocate(sizeof(byte) * imageWidth * 4);

  if (!scanline)
  {
    zDeallocate(outputImage);
    status.SetFailed("Failed to allocate memory for the Hdr scanline");
    return;
  }

  // Process one scanline at a time
  for (uint line = 0; line < imageHeight; ++line)
  {
    if (!DecodeHdrScanline(imageData, endData, imageWidth, scanline))
    {
      status.SetFailed("Corrupted or invalid image data");
      zDeallocate(scanline);
      zDeallocate(outputImage);
      return;
    }

    // Convert each pixel from the decoded scanline
    float* outputScanline = outputImage + line * imageWidth * 3;
    for (uint pixel = 0; pixel < imageWidth; ++pixel)
      RgbeToRgb32f(scanline + pixel * 4, outputScanline + pixel * 3);
  }

  zDeallocate(scanline);
  *width = imageWidth;
  *height = imageHeight;
  *output = (byte*)outputImage;

  // We always output the RGB32f format.
  *format = TextureFormat::RGB32f;
}

void SaveHdr(Status& status, Stream* stream, const byte* image, uint width, uint height, TextureFormat::Enum format)
{
  if (!IsHdrSaveFormat(format))
  {
    status.SetFailed("Hdr only supports the format RGB32f");
    return;
  }

  if (image == nullptr || width == 0 || height == 0)
  {
    status.SetFailed("Empty Image");
    return;
  }

  uint dataSize = width * height * 4;
  byte* outputData = new byte[dataSize];

  for (uint i = 0; i < width * height; ++i)
    Rgb32fToRgbe((float*)image + i * 3, outputData + i * 4);

  // Build header
  StringBuilder header;
  header.Append("#?RADIANCE\n");
  header.Append("FORMAT=32-bit_rle_rgbe\n");
  header.Append("\n");
  header.Append(String::Format("-Y %d +X %d\n", height, width));

  // Write header
  uint headerSize = header.GetSize();
  byte* headerData = new byte[headerSize];
  header.ExtractInto(headerData, headerSize);
  stream->Write(headerData, headerSize);
  delete[] headerData;

  // Write image data
  stream->Write(outputData, dataSize);
}

void RgbeToRgb32f(byte* rgbe, float* rgb32f)
{
  byte e = rgbe[3];
  if (e != 0)
  {
    float exp = ldexp(1.0f / 256.0f, (int)e - 128);
    rgb32f[0] = rgbe[0] * exp;
    rgb32f[1] = rgbe[1] * exp;
    rgb32f[2] = rgbe[2] * exp;
  }
  else
  {
    rgb32f[0] = 0.0f;
    rgb32f[1] = 0.0f;
    rgb32f[2] = 0.0f;
  }
}

void Rgb32fToRgbe(float* rgb32f, byte* rgbe)
{
  float maxValue = rgb32f[0];
  if (rgb32f[1] > maxValue)
    maxValue = rgb32f[1];
  if (rgb32f[2] > maxValue)
    maxValue = rgb32f[2];

  if (maxValue < 1e-32)
  {
    rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
  }
  else
  {
    int exp;
    float scale = frexp(maxValue, &exp) * 256.0f / maxValue;
    rgbe[0] = (byte)(rgb32f[0] * scale);
    rgbe[1] = (byte)(rgb32f[1] * scale);
    rgbe[2] = (byte)(rgb32f[2] * scale);
    rgbe[3] = (byte)(exp + 128);
  }
}

} // namespace Raverie
