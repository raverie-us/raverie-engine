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

#include "CodeRangeMapping.hpp"
#include "ShaderAttributes.hpp"
#include "ShaderType.hpp"
#include "ShaderLibrary.hpp"
#include "ShaderErrors.hpp"
#include "ShaderProject.hpp"
#include "ShaderCodeBuilder.hpp"
#include "LibraryTranslator.hpp"
#include "ZilchShaderSharedSettings.hpp"
#include "BaseShaderTranslator.hpp"
#include "Translator.hpp"
#include "TranslatorContext.hpp"
#include "GenericTranslation.hpp"
#include "GlslTranslation.hpp"
#include "GlslTranslator.hpp"

#include "spirv.hpp"
#include "SpirVHelpers.hpp"
#include "ZilchShaderIRMeta.hpp"
#include "ZilchShaderIRReflection.hpp"
#include "ZilchShaderIRShared.hpp"
#include "ZilchSpirVSettings.hpp"
#include "SimpleZilchParser.hpp"
#include "ExtensionLibrary.hpp"
#include "OperatorKeys.hpp"
#include "ZilchShaderIRLibrary.hpp"
#include "ZilchShaderIRProject.hpp"
#include "BaseShaderIRTranslator.hpp"
#include "ShaderIRLibraryTranslation.hpp"
#include "StageRequirementsGatherer.hpp"
#include "ZilchSpirVFrontEnd.hpp"
#include "ZilchShaderIRCore.hpp"
#include "ZilchShaderIntrinsics.hpp"
#include "EntryPointGeneration.hpp"
#include "ZilchSpirVFrontEndValidation.hpp"
#include "ShaderByteStream.hpp"
#include "ZilchShaderIRTranslationPass.hpp"
#include "ZilchSpirVDisassemblerBackend.hpp"
#include "ZilchShaderGlslBackend.hpp"
#include "ZilchShaderIRPasses.hpp"
#include "SpirVSpecializationConstantPass.hpp"
#include "TypeDependencyCollector.hpp"
#include "ZilchShaderSpirVBinaryBackend.hpp"
#include "ZilchShaderIRCompositor.hpp"
#include "SimpleZilchShaderIRGenerator.hpp"

#include "LibraryTranslationHelpers.hpp"
#include "ShaderImageIntrinsics.hpp"
#include "ShaderIntrinsicTypes.hpp"
#include "ZilchTypeCollector.hpp"
#include "Compositor.hpp"
#include "SimpleZilchShaderGenerator.hpp"
#include "ZilchShaderSettingsLibrary.hpp"

namespace Zilch
{

ZilchDeclareStaticLibrary(ShaderIntrinsicsLibrary, ZilchNoNamespace, ZeroNoImportExport);

}//namespace Zilch
