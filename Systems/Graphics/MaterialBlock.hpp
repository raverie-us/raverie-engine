// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#pragma once

namespace Zero
{

/// Represents meta data of shader fragment types that can be composited together on Materials.
class MaterialBlock
{
public:
  ZilchDeclareInheritableType(MaterialBlock, TypeCopyMode::ReferenceType);

  MaterialBlock();
  virtual ~MaterialBlock() {}

  virtual void Serialize(Serializer& stream);

  Material* mOwner;

  IndexRange AddShaderInputs(Array<ShaderInput>& shaderInputs);
};

typedef HandleOf<MaterialBlock> MaterialBlockHandle;

} // namespace Zero
