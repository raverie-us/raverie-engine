///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Zilch/Zilch.hpp"

#include "ForwardDeclarations.hpp"

#include "ShaderAttributes.hpp"
#include "ShaderErrors.hpp"
#include "ShaderCodeBuilder.hpp"

#include "spirv.hpp"
#include "SpirVHelpers.hpp"
#include "ZilchShaderIRMeta.hpp"
#include "ZilchShaderIRReflection.hpp"
#include "ZilchShaderIRShared.hpp"
#include "ZilchSpirVSettings.hpp"
#include "ExtensionLibrary.hpp"
#include "OperatorKeys.hpp"
#include "ShaderIRLibraryTranslation.hpp"
#include "ZilchShaderIRLibrary.hpp"
#include "ZilchShaderIRProject.hpp"
#include "ZilchShaderIRCore.hpp"
#include "LibraryTranslationHelpers.hpp"
#include "ShaderImageIntrinsics.hpp"
#include "ShaderIntrinsicTypes.hpp"

#include "BaseShaderIRTranslator.hpp"
#include "ShaderIntrinsicsStaticZilchLibrary.hpp"
#include "StageRequirementsGatherer.hpp"
#include "SimpleZilchParser.hpp"
#include "EntryPointGeneration.hpp"
#include "ZilchSpirVFrontEndValidation.hpp"
#include "ZilchSpirVFrontEnd.hpp"

#include "TypeDependencyCollector.hpp"
#include "ShaderByteStream.hpp"
#include "ZilchShaderIRTranslationPass.hpp"
#include "ZilchShaderIRPasses.hpp"
#include "ZilchShaderSpirVBinaryBackend.hpp"
#include "ZilchSpirVDisassemblerBackend.hpp"
#include "ZilchShaderGlslBackend.hpp"
#include "SpirVSpecializationConstantPass.hpp"
#include "ZilchShaderIRCompositor.hpp"
#include "SimpleZilchShaderIRGenerator.hpp"

namespace Zilch
{

ZilchDeclareStaticLibrary(ShaderIntrinsicsLibrary, ZilchNoNamespace, ZeroNoImportExport);

}//namespace Zilch

namespace Zero
{

ZilchDeclareStaticLibrary(ShaderSettingsLibrary, ZilchNoNamespace, ZeroNoImportExport);

}//namespace Zero
