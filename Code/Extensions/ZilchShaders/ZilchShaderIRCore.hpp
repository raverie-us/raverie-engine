// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

/// The zilch shader wrapper around Zilch's core library. This needs to be
/// built and have the Parse function called once before all shader translation.
/// Contains the primitive types for translation.
class ZilchShaderIRCore
{
public:
  static void InitializeInstance();
  static void Destroy();
  static ZilchShaderIRCore& GetInstance();

  ZilchShaderIRCore();
  /// Parse the core library and make all backing shader types.
  void Parse(ZilchSpirVFrontEnd* translator);
  ZilchShaderIRLibraryRef GetLibrary();

  ZilchTypeGroups mZilchTypes;
  ShaderTypeGroups mShaderTypes;
  SpirVExtensionLibrary* mGlsl450ExtensionsLibrary;

private:
  void MakeMathTypes(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types);
  void RegisterPrimitiveFunctions(ZilchSpirVFrontEnd* translator,
                                  ZilchShaderIRLibrary* shaderLibrary,
                                  ShaderTypeGroups& types,
                                  ZilchShaderIRType* shaderType);
  void RegisterVectorFunctions(ZilchSpirVFrontEnd* translator,
                               ZilchShaderIRLibrary* shaderLibrary,
                               ShaderTypeGroups& types,
                               Array<ZilchShaderIRType*>& vectorTypes);
  void RegisterMatrixFunctions(ZilchSpirVFrontEnd* translator,
                               ZilchShaderIRLibrary* shaderLibrary,
                               ShaderTypeGroups& types,
                               Array<ZilchShaderIRType*>& matrixTypes);
  void RegisterQuaternionFunctions(ZilchSpirVFrontEnd* translator,
                                   ZilchShaderIRLibrary* shaderLibrary,
                                   ShaderTypeGroups& types,
                                   ZilchShaderIRType* quaternionType);

  ZilchShaderIRLibraryRef mLibraryRef;
  static ZilchShaderIRCore* mInstance;
};

} // namespace Zero
