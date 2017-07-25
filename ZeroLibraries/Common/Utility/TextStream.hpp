///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "String/StringBuilder.hpp"

namespace Zero
{

// Simple callback interface for process stdio
class TextStream
{
public:
  virtual void Write(cstr text)=0;
  virtual ~TextStream(){};
};

class TextStreamDebugPrint : public TextStream
{
  virtual void Write(cstr text)
  {
    ZPrintFilter(Filter::DefaultFilter, "%s", text);
  }
};

class TextStreamBuffer : public TextStream
{
public:
  StringBuilder buffer;
  String ToString(){return buffer.ToString();}
  void Write(cstr text) override { buffer.Append(text);}
};

class TextStreamNull : public TextStream
{
  void Write(cstr text) override {};
};


}
