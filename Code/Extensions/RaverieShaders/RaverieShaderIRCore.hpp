// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// The raverie shader wrapper around Raverie's core library. This needs to be
/// built and have the Parse function called once before all shader translation.
/// Contains the primitive types for translation.
class RaverieShaderIRCore
{
public:
  static void InitializeInstance();
  static void Destroy();
  static RaverieShaderIRCore& GetInstance();

  RaverieShaderIRCore();
  /// Parse the core library and make all backing shader types.
  void Parse(RaverieSpirVFrontEnd* translator);
  RaverieShaderIRLibraryRef GetLibrary();

  RaverieTypeGroups mRaverieTypes;
  ShaderTypeGroups mShaderTypes;
  SpirVExtensionLibrary* mGlsl450ExtensionsLibrary;

private:
  void MakeMathTypes(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types);
  void RegisterPrimitiveFunctions(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types, RaverieShaderIRType* shaderType);
  void RegisterVectorFunctions(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types, Array<RaverieShaderIRType*>& vectorTypes);
  void RegisterMatrixFunctions(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types, Array<RaverieShaderIRType*>& matrixTypes);
  void RegisterQuaternionFunctions(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types, RaverieShaderIRType* quaternionType);

  RaverieShaderIRLibraryRef mLibraryRef;
  static RaverieShaderIRCore* mInstance;
};

} // namespace Raverie
