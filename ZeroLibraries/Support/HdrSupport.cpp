#include "Precompiled.hpp"

namespace
{

bool FindNewline(cstr data, uint size, char* newline)
{
  uint i = 0;
  while (i < size)
  {
    if (data[i] == '\r')
    {
      newline[0] = '\r';
      if (i + 1 < size && data[i + 1] == '\n')
        newline[1] = '\n';
      return true;
    }
    else if (data[i] == '\n')
    {
      newline[0] = '\n';
      return true;
    }
    ++i;
  }

  return false;
}

uint ReadLine(cstr data, uint size, cstr newline, Zero::String& line)
{
  uint i = 0;
  // Find newline location
  while (i < size)
  {
    if (data[i] == newline[0])
    {
      if (newline[1] == 0)
        break;
      else if (i + 1 < size && data[i + 1] == newline[1])
        break;
    }
    ++i;
  }

  // No data or didn't find newline
  if (i == size)
    return 0;

  line = Zero::String(data, i);

  // Return the index passed the newline
  i += (newline[1] == 0) ? 1 : 2;
  return i;
}

}

namespace Zero
{

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
  GrammarRule<Character>& comment    = mTokenGrammar["Comment"];
  GrammarRule<Character>& whitespace = mTokenGrammar["Whitespace"];
  GrammarRule<Character>& token      = mTokenGrammar["Token"];
  GrammarRule<Character>& assignment = mTokenGrammar["Assignment"];
  GrammarRule<Character>& sign       = mTokenGrammar["Sign"];

  tokenStart |= comment | whitespace | token | assignment | sign;
  comment    |= T("#") << *T("^\r\n");
  whitespace |= *T(" \t\r\n");
  token      |= T("a-zA-Z_0-9") << *T("a-zA-Z_0-9");
  assignment |= T("=") << *T("^\r\n");
  sign       |= T("+-");

  mTokenGrammar.AddIgnore(whitespace);

  GrammarRule<Token>& start      = mParserGrammar["Start"];
  GrammarRule<Token>& statement  = mParserGrammar["Statement"];
  GrammarRule<Token>& variable   = mParserGrammar["Variable"];
  GrammarRule<Token>& dimensions = mParserGrammar["Dimensions"];

  start      |= P("FirstComment", P(comment)) << *statement << dimensions;
  statement  |= P(comment) | variable;
  variable   |= P("Name", P(token)) << P("Value", P(assignment));
  dimensions |= P("YSign", P(sign)) << P("YName", P(token)) << P("YDim", P(token)) << P("XSign", P(sign)) << P("XName", P(token)) << P("XDim", P(token));

  mTokenStart = &tokenStart;
  mStartRule = &start;
}

class HdrHeaderParser
{
public:
  static bool Parse(Status& status, StringParam header, uint& width, uint& height);

  // Only grabbing values on EndRule
  void StartRule(GrammarRule<Token>* rule) {}
  void EndRule(ParseNodeInfo<Token>* info);
  void TokenParsed(ParseNodeInfo<Token>* info) {}
  void StartParsing() {}
  void EndParsing() {}

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

const byte* ParseHdrHeader(Status& status, const byte* data, uint size, uint& width, uint& height)
{
  cstr headerData = (cstr)data;
  uint i = 0;

  // Determine newline type
  char newline[2] = {0};
  if (FindNewline(headerData, size, newline) == false)
  {
    status.SetFailed("No header found");
    return nullptr;
  }

  // Find empty line for end of header
  while (i < size)
  {
    String line;
    uint index = ReadLine(headerData + i, size - i, newline, line);
    if (index == 0)
    {
      status.SetFailed("No end of header found (empty line)");
      return nullptr;
    }

    i += index;
    if (line.Empty())
      break;
  }

  // Read past resolution line
  String line;
  i += ReadLine(headerData + i, size - i, newline, line);
  if (i == size)
  {
    status.SetFailed("No resolution line after header");
    return nullptr;
  }

  String header(headerData, i);
  if (!HdrHeaderParser::Parse(status, header, width, height))
    return nullptr;

  // Return data pointer after the header (the image data)
  return data + i;
}

// Used to make sure no data reads go out of bounds
#define ValidateRead(data, endData, count) if (data + (count) > endData) return false;

bool DecodeHdrScanline(const byte*& imageData, const byte* endData, uint imageWidth, byte* scanline)
{
  uint index = 0;
  ValidateRead(imageData, endData, index + 4);

  byte byte0 = imageData[index++];
  byte byte1 = imageData[index++];
  byte byte2 = imageData[index++];
  byte byte3 = imageData[index++];

  uint scanWidth = (uint)byte2 << 8 | byte3;

  // If first two bytes are of value 2 and next two are the high and low bytes of the scanline width
  // then scanline is adaptive run length encoded
  if (byte0 == 2 && byte1 == 2 && scanWidth == imageWidth)
  {
    // All four channels (R, G, B, E) are stored separately in this encoding
    for (uint channel = 0; channel < 4; ++channel)
    {
      uint pixel = 0;
      while (pixel < scanWidth)
      {
        ValidateRead(imageData, endData, index + 2);

        // A value greater than 128 means a run of the same value (and actual count is without the high bit),
        // less than 128 means a run of different values, equal to 128 means nothing and is invalid
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
  if (rgb32f[1] > maxValue) maxValue = rgb32f[1];
  if (rgb32f[2] > maxValue) maxValue = rgb32f[2];

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

void LoadFromHdr(Status& status, byte** output, uint* width, uint* height, const byte* data, uint size)
{
  uint imageWidth, imageHeight;
  const byte* imageData = ParseHdrHeader(status, data, size, imageWidth, imageHeight);
  if (imageData == nullptr)
    return;

  // Subtract header size
  size -= imageData - data;
  const byte* endData = imageData + size;

  // Allocate full size of final image
  float* outputImage = new float[imageWidth * imageHeight * 3];
  // Scratch buffer for decoding a single scanline
  byte* scanline = new byte[imageWidth * 4];

  // Process one scanline at a time
  for (uint line = 0; line < imageHeight; ++line)
  {
    if (!DecodeHdrScanline(imageData, endData, imageWidth, scanline))
    {
      status.SetFailed("Corrupted or invalid image data");
      delete[] scanline;
      delete[] outputImage;
      return;
    }

    // Convert each pixel from the decoded scanline
    float* outputScanline = outputImage + line * imageWidth * 3;
    for (uint pixel = 0; pixel < imageWidth; ++pixel)
      RgbeToRgb32f(scanline + pixel * 4, outputScanline + pixel * 3);
  }

  delete[] scanline;
  *width = imageWidth;
  *height = imageHeight;
  *output = (byte*)outputImage;
}

void SaveToHdr(Status& status, byte* image, uint width, uint height, StringParam filename)
{
  if (image == nullptr || width == 0 || height == 0)
  {
    status.SetFailed("Empty Image");
    return;
  }

  File file;
  file.Open(filename.c_str(), FileMode::Write, FileAccessPattern::Sequential);

  if (!file.IsOpen())
  {
    status.SetFailed(String::Format("Can not open hdr file '%s' for writing", filename.c_str()));
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
  file.Write(headerData, headerSize);
  delete[] headerData;

  // Write image data
  file.Write(outputData, dataSize);

  file.Close();
}

} // namespace Zero
