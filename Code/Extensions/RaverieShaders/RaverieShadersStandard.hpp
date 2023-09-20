// MIT Licensed (see LICENSE.md).
#pragma once

#include "Foundation/Common/CommonStandard.hpp"
#include "Foundation/RaverieLanguage/Precompiled.hpp"

#include "ForwardDeclarations.hpp"

#include "ShaderAttributes.hpp"
#include "ShaderErrors.hpp"
#include "ShaderCodeBuilder.hpp"

// Grab the latest unified spirv file. Update when switching spirv versions
#include "spirv/unified1/spirv.hpp"
#include "SpirVHelpers.hpp"
#include "RaverieShaderIRMeta.hpp"
#include "RaverieShaderIRReflection.hpp"
#include "RaverieShaderIRShared.hpp"
#include "RaverieShaderIRExtendedTypes.hpp"
#include "RaverieSpirVSettings.hpp"
#include "ExtensionLibrary.hpp"
#include "OperatorKeys.hpp"
#include "ShaderIRLibraryTranslation.hpp"
#include "RaverieShaderIRLibrary.hpp"
#include "RaverieShaderIRProject.hpp"
#include "RaverieShaderIRCore.hpp"
#include "LibraryTranslationHelpers.hpp"
#include "CommonInstructions.hpp"
#include "ShaderImageIntrinsics.hpp"
#include "ShaderIntrinsicTypes.hpp"

#include "BaseShaderIRTranslator.hpp"
#include "ShaderIntrinsicsStaticRaverieLibrary.hpp"
#include "CycleDetection.hpp"
#include "StageRequirementsGatherer.hpp"
#include "SimpleRaverieParser.hpp"
#include "EntryPointGeneration.hpp"
#include "RaverieSpirVFrontEndValidation.hpp"
#include "RaverieSpirVFrontEnd.hpp"

#include "TypeDependencyCollector.hpp"
#include "ShaderByteStream.hpp"
#include "RaverieShaderIRTranslationPass.hpp"
#include "RaverieShaderIRPasses.hpp"
#include "RaverieShaderSpirVBinaryBackend.hpp"
#include "RaverieSpirVDisassemblerBackend.hpp"
#include "RaverieShaderGlslBackend.hpp"
#include "SpirVSpecializationConstantPass.hpp"
#include "RaverieShaderIRCompositor.hpp"
#include "SimpleRaverieShaderIRGenerator.hpp"

namespace Raverie
{

RaverieDeclareStaticLibrary(ShaderIntrinsicsLibrary);

} // namespace Raverie

namespace Raverie
{

RaverieDeclareStaticLibrary(ShaderSettingsLibrary);

} // namespace Raverie
