///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderIRCore
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

  TypeGroups mTypes;
  SpirVExtensionLibrary* mGlsl450ExtensionsLibrary;

private:
  void MakeMathTypes(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types);
  void RegisterPrimitiveFunctions(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types, ZilchShaderIRType* shaderType);
  void RegisterVectorFunctions(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types, Array<ZilchShaderIRType*>& vectorTypes);
  void RegisterMatrixFunctions(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types, Array<ZilchShaderIRType*>& matrixTypes);
  void RegisterQuaternionFunctions(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types, ZilchShaderIRType* quaternionType);

  ZilchShaderIRLibraryRef mLibraryRef;
  static ZilchShaderIRCore* mInstance;
};

}//namespace Zero
