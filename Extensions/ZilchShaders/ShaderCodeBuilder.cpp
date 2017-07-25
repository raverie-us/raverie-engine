///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ShaderCodeBuilder
ShaderCodeBuilder::ShaderCodeBuilder()
  : LineReturn("\n"), Space(" "), SingleIndentation("  ")
{
  mScopes = 0;
}

void ShaderCodeBuilder::BeginScope()
{
  WriteIndentation();
  WriteLine("{");
  ++mScopes;
}

void ShaderCodeBuilder::EndScope()
{
  --mScopes;
  WriteIndentation();
  WriteLine("}");
}

void ShaderCodeBuilder::EndClassScope()
{
  --mScopes;
  WriteIndentation();
  WriteLine("};");
}

ShaderCodeBuilder& ShaderCodeBuilder::operator<<(StringParam string)
{
  Write(string);
  return *this;
}

ShaderCodeBuilder& ShaderCodeBuilder::operator<<(ShaderCodeBuilder& builder)
{
  // If this is another builder then write its contents, otherwise just return ourself
  // (this kind of insertion is just meant to make certain functions easier to chain)
  if(&builder != this)
    Write(builder.ToString());
  return *this;
}

ShaderCodeBuilder& ShaderCodeBuilder::operator<<(char value)
{
  mBuilder.Append(value);
  return *this;
}

ShaderCodeBuilder& ShaderCodeBuilder::operator<<(Rune value)
{
  mBuilder.Append(value);
  return *this;
}

String ShaderCodeBuilder::EmitLineReturn()
{
  return LineReturn;
}

String ShaderCodeBuilder::EmitSpace()
{
  return Space;
}

String ShaderCodeBuilder::EmitIndent()
{
  StringBuilder builder;
  builder.Repeat(mScopes, SingleIndentation);
  return builder.ToString();
}

ShaderCodeBuilder& ShaderCodeBuilder::WriteScopedIndent()
{
  WriteIndentation();
  return *this;
}

void ShaderCodeBuilder::WriteIndent()
{
  mBuilder.Append("  ");
}

void ShaderCodeBuilder::WriteIndentation()
{
  mBuilder.Repeat(mScopes, SingleIndentation);
}

void ShaderCodeBuilder::WriteSpace()
{
  mBuilder.Append(" ");
}

void ShaderCodeBuilder::Write(StringParam string)
{
  mBuilder.Append(string);
}

void ShaderCodeBuilder::WriteLine()
{
  mBuilder.Append("\n");
}

void ShaderCodeBuilder::Clear()
{
  mScopes = 0;
  mBuilder.Deallocate();
}

String ShaderCodeBuilder::ToString()
{
  return mBuilder.ToString();
}

size_t ShaderCodeBuilder::GetSize()
{
  return mBuilder.GetSize();
}

void ShaderCodeBuilder::WriteLine(StringParam string)
{
  mBuilder.Append(string);
  WriteLine();
}

//-------------------------------------------------------------------ScopedShaderCodeBuilder
ScopedShaderCodeBuilder::ScopedShaderCodeBuilder(ZilchShaderTranslatorContext* context)
{
  mContext = context;
  mContext->PushBuilder(this);
}

ScopedShaderCodeBuilder::~ScopedShaderCodeBuilder()
{
  PopFromStack();
}

void ScopedShaderCodeBuilder::PopFromStack()
{
  if(mContext != nullptr)
    mContext->PopBuilder();
  mContext = nullptr;
}

}//namespace Zero
