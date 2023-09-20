// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

ShaderCodeBuilder::ShaderCodeBuilder() : LineReturn("\n"), Space(" "), SingleIndentation("  ")
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
  // If this is another builder then write its contents, otherwise just return
  // ourself (this kind of insertion is just meant to make certain functions
  // easier to chain)
  if (&builder != this)
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

ShaderCodeBuilder& ShaderCodeBuilder::WriteMemberVariableDeclaration(StringParam variableName, StringParam variableTypeName)
{
  (*this) << "var " << variableName << " : " << variableTypeName << ";" << this->EmitLineReturn();
  return *this;
}

ShaderCodeBuilder& ShaderCodeBuilder::WriteVariableDeclaration(ShaderIRAttributeList& attributes, StringParam variableName, StringParam variableTypeName)
{
  ShaderCodeBuilder& builder = *this;
  builder << builder.EmitIndent();
  // Add all attributes
  ShaderIRAttributeList::Range attributeRange = attributes.All();
  for (; !attributeRange.Empty(); attributeRange.PopFront())
  {
    ShaderIRAttribute& attribute = attributeRange.Front();
    builder.DeclareAttribute(attribute);
  }

  // Declare the field
  builder << builder.EmitSpace();
  builder.WriteMemberVariableDeclaration(variableName, variableTypeName);
  return builder;
}

ShaderCodeBuilder& ShaderCodeBuilder::WriteVariableDeclaration(ShaderIRAttribute& attribute, StringParam variableName, StringParam variableTypeName)
{
  ShaderCodeBuilder& builder = *this;
  builder << builder.EmitIndent();
  builder.DeclareAttribute(attribute);
  builder << builder.EmitSpace();
  // Declare the field
  builder.WriteMemberVariableDeclaration(variableName, variableTypeName);
  return builder;
}

ShaderCodeBuilder& ShaderCodeBuilder::WriteLocalVariableDefaultConstruction(StringParam variableName, StringParam variableTypeName)
{
  (*this) << this->EmitIndent() << "var " << variableName << " = " << variableTypeName << "();" << this->EmitLineReturn();
  return *this;
}

ShaderCodeBuilder& ShaderCodeBuilder::DeclareAttribute(ShaderIRAttribute& attribute)
{
  ShaderCodeBuilder& builder = *this;
  // Add all attributes
  builder << "[" << attribute.mAttributeName;
  // Write any attribute parameters that exist
  DeclareAttributeParams(attribute);
  builder << "]";
  return builder;
}

ShaderCodeBuilder& ShaderCodeBuilder::DeclareAttribute(StringParam attributeName)
{
  ShaderCodeBuilder& builder = *this;
  // Add all attributes
  builder << "[" << attributeName << "]";
  return builder;
}

ShaderCodeBuilder& ShaderCodeBuilder::DeclareAttributeParams(ShaderIRAttribute& attribute)
{
  ShaderCodeBuilder& builder = *this;
  size_t paramCount = attribute.mParameters.Size();
  // There's no params so do nothing
  if (paramCount == 0)
    return builder;

  // Write each attribute out (assuming only string value right now)
  builder << "(";
  for (size_t i = 0; i < paramCount; ++i)
  {
    ShaderIRAttributeParameter& param = attribute.mParameters[i];
    // If this parameter has no name then just write the value
    if (param.GetName().Empty())
      builder << param.GetStringValue();
    // Otherwise write 'name' : 'value'
    else
      builder << param.GetName() << " : "
              << "\"" << param.GetStringValue() << "\"";
    if (i != paramCount - 1)
      builder << ", ";
  }
  builder << ")";
  return builder;
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

} // namespace Raverie
