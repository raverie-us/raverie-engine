#include "Precompiled.hpp"

uint mSeed = 3469287143;
static const uint cRandMax = 0x7FFF;

uint RandomInteger(void)
{
  mSeed = 214013 * mSeed + 2531011;
  return (mSeed >> 16) & cRandMax;
}

bool Chance(float chance)
{
  uint rand = RandomInteger();
  float norm = rand / (float)cRandMax;
  return norm < chance;
}

String GeneratePotentiallyNonTokenizable()
{
  uint length = RandomInteger() % 10000;

  StringBuilder result;

  char valid[] =
  {
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    '~','!','@','#','$','%','^','&','*','(',')','-','+','=','[',']',
    '{','}','\\','/','"','\'',';',':','<','>',',','.','?','|',
    '0','1','2','3','4','5','6','7','8','9',' ','\t','\r','\n'
  };

  for (size_t i = 0; i < length; ++i)
  {
    if (Chance(0.9f))
    {
      result.Append(valid[RandomInteger() % sizeof(valid)]);
    }
    else
    {
      char c = (char)RandomInteger() % 256;
      // Do NOT emit the bell character, otherwise the unit tests make sound and its annoying!
      if (c == '\a')
      {
        result.Append(valid[RandomInteger() % sizeof(valid)]);
      }
      else
      {
        result.Append(c);
      }
    }
  }

  return result.ToString();
}

String GenerateTokenizablePotentiallyNonParsable()
{
  uint length = RandomInteger() % 1000;

  StringBuilder result;

  char identifierStart[] =
  {
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
  };

  char identifierEnd[] =
  {
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    '0','1','2','3','4','5','6','7','8','9'
  };

  char numeric[] =
  {
    '0','1','2','3','4','5','6','7','8','9'
  };

  //char characters[] =
  //{
  //  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
  //  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  //  '~','!','@','#','$','%','^','&','*','(',')','-','+','=','[',']',
  //  '{','}','\\','/','\'',';',':','<','>',',','.','?','|','"',
  //  '0','1','2','3','4','5','6','7','8','9',' ','\t','\r','\n'
  //};
  
  char stringCharacters[] =
  {
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    '~','!','@','#','$','%','^','&','*','(',')','-','+','=','[',']',
    '{','}','/','\'',';',':','<','>',',','.','?','|',
    '0','1','2','3','4','5','6','7','8','9',' ','\t'
  };

  char whitespace[] =
  {
    ' ','\t','\r','\n'
  };

  const char* tokens[] =
  {
    "abstract",       "sizeof",
    "alias",          "stackalloc",
    "alignof",        "static",
    "as",             "struct",
    "assert",         "switch",
    "auto",           "throw",
    "base",           "true",
    "break",          "try",
    "case",           "typedef",
    "catch",          "typeid",
    "checked",        "typename",
    "class",          "typeof",
    "const",          "type",
    "constructor",    "unchecked",
    "continue",       "unsafe",
    "default",        "unsigned",
    "delegate",       "using",
    "delete",         "var",
    "destructor",     "virtual",
    "do",             "volatile",
    "dynamic",        "where",
    "else",           "while",
    "enum",           "yield",
    "event",          ".",
    "explicit",       "->",
    "export",         ":",
    "extern",         ",",
    "false",          "=>",
    "finally",        ":=",
    "fixed",          "-=",
    "for",            "+=",
    "foreach",        "/=",
    "friend",         "*=",
    "function",       "%=",
    "get",            "^=",
    "global",         "<<=",
    "goto",           ">>=",
    "if",             "$=",
    "immutable",      "|=",
    "implicit",       "&=",
    "import",         "==",
    "in",             "!=",
    "include",        "<",
    "inline",         "<=",
    "interface",      ">",
    "internal",       ">=",
    "is",             "-",
    "local",          "+",
    "lock",           "/",
    "loop",           "*",
    "module",         "%",
    "mutable",        "^",
    "namespace",      "--",
    "new",            "++",
    "null",           "<<",
    "operator",       ">>",
    "out",            "$",
    "override",       "|",
    "package",        "&",
    "params",         "~",
    "partial",        "||",
    "positional",     "&&",
    "private",        "!",
    "protected",      ";",
    "public",         "[",
    "readonly",       "]",
    "ref",            "(",
    "register",       ")",
    "require",        "{",
    "return",         "}"//,
    "sealed",         //"//",
    "set",            //"/*",
    "signed",         //"*/"
  };

#define count(a) (sizeof(a) / sizeof(a[0]))

  for (size_t i = 0; i < length; ++i)
  {
    if (Chance(0.3f))
    {
      result.Append(tokens[RandomInteger() % count(tokens)]);
    }
    else if (Chance(0.3f))
    {
      result.Append(whitespace[RandomInteger() % count(whitespace)]);
    }
    else if (Chance(0.3f))
    {
      uint identifierLength = RandomInteger() % 32;

      for (size_t j = 0; j < identifierLength; ++j)
      {
        if (j == 0)
        {
          result.Append(identifierStart[RandomInteger() % count(identifierStart)]);
        }
        else
        {
          result.Append(identifierEnd[RandomInteger() % count(identifierEnd)]);
        }
      }
    }
    else if (Chance(0.3f))
    {
      bool isDecimal = false;
      uint numericLength = RandomInteger() % 32;

      for (size_t j = 0; j < numericLength; ++j)
      {
        result.Append(numeric[RandomInteger() % count(numeric)]);

        if (Chance(0.02f) && j != numericLength - 1)
        {
          result.Append(".");
          isDecimal = true;
        }
      }
    }
    else
    {
      uint literalLength = RandomInteger() % 100;

      result.Append("ddd\"");

      for (size_t j = 0; j < literalLength; ++j)
      {
        result.Append(stringCharacters[RandomInteger() % count(stringCharacters)]);
      }

      result.Append("\"eee");
    }

    
    result.Append(whitespace[RandomInteger() % count(whitespace)]);
  }

  return result.ToString();
}

String GenerateParsablePotentiallyNonSyntaxable()
{
  return "";
}

String GenerateCorrect()
{
  return "";
}
