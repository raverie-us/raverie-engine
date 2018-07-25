// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#pragma once

namespace Zero
{

/// Represents meta data of shader fragment types that can be composited together on Materials.
class MaterialBlock : public IZilchObject
{
public:
  ZilchDeclareInheritableType(TypeCopyMode::ReferenceType);

  MaterialBlock();
  virtual ~MaterialBlock() {}

  virtual void Serialize(Serializer& stream);

  Material* mOwner;

  void MarkAsModified();
  IndexRange AddShaderInputs(Array<ShaderInput>& shaderInputs);
};

typedef HandleOf<MaterialBlock> MaterialBlockHandle;

// Meta functions for shader fragments that inherit from MaterialBlock.

// Constructor/Destructor for fragments being created in a ZilchScript.
void FragmentConstructor(Call& call, ExceptionReport& report);
void FragmentDestructor(Call& call, ExceptionReport& report);

// Getter/Setter for primitive types that are directly memory copyable.
void FragmentGetter(Call& call, ExceptionReport& report);
void FragmentSetter(Call& call, ExceptionReport& report);

// Getter/Setter for Texture handles.
void FragmentTextureGetter(Call& call, ExceptionReport& report);
void FragmentTextureSetter(Call& call, ExceptionReport& report);

} // namespace Zero
