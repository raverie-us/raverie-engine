add_library(RaverieShaders)

raverie_setup_library(RaverieShaders ${CMAKE_CURRENT_LIST_DIR} TRUE)

target_sources(RaverieShaders
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/ArithmeticInstructions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BaseShaderIRTranslator.hpp
    ${CMAKE_CURRENT_LIST_DIR}/BitInstructions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CommonInstructions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CommonInstructions.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ConversionInstructions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CycleDetection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CycleDetection.hpp
    ${CMAKE_CURRENT_LIST_DIR}/EntryPointGeneration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/EntryPointGeneration.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ExtensionLibrary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ExtensionLibrary.hpp
    ${CMAKE_CURRENT_LIST_DIR}/FixedArray.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ForwardDeclarations.hpp
    ${CMAKE_CURRENT_LIST_DIR}/GLSL.std.450.h
    ${CMAKE_CURRENT_LIST_DIR}/Glsl450Extensions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LibraryTranslationHelpers.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LibraryTranslationHelpers.hpp
    ${CMAKE_CURRENT_LIST_DIR}/LogicalInstructions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OperatorKeys.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OperatorKeys.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderAttributes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderAttributes.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderByteStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderByteStream.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderCodeBuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderCodeBuilder.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderErrors.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderErrors.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderIRLibraryTranslation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderIRLibraryTranslation.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderImageIntrinsics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderImageIntrinsics.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderIntrinsicTypes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderIntrinsicTypes.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderIntrinsics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderIntrinsicsStaticRaverieLibrary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderIntrinsicsStaticRaverieLibrary.hpp
    ${CMAKE_CURRENT_LIST_DIR}/SimpleRaverieParser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SimpleRaverieParser.hpp
    ${CMAKE_CURRENT_LIST_DIR}/SimpleRaverieShaderIRGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SimpleRaverieShaderIRGenerator.hpp
    ${CMAKE_CURRENT_LIST_DIR}/SpirVHelpers.hpp
    ${CMAKE_CURRENT_LIST_DIR}/SpirVSpecializationConstantPass.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SpirVSpecializationConstantPass.hpp
    ${CMAKE_CURRENT_LIST_DIR}/StageRequirementsGatherer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StageRequirementsGatherer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/TypeDependencyCollector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TypeDependencyCollector.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderGlslBackend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderGlslBackend.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRCompositor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRCompositor.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRCore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRCore.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRExtendedTypes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRExtendedTypes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRLibrary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRLibrary.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRMeta.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRMeta.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRPasses.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRPasses.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRProject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRProject.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRReflection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRReflection.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRShared.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRShared.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRTranslationPass.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderIRTranslationPass.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderSpirVBinaryBackend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShaderSpirVBinaryBackend.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShadersStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieShadersStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieSpirVDisassemblerBackend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieSpirVDisassemblerBackend.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieSpirVFrontEnd.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieSpirVFrontEnd.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieSpirVFrontEndValidation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieSpirVFrontEndValidation.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieSpirVSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieSpirVSettings.hpp
)

raverie_target_includes(RaverieShaders
  PUBLIC
    Common
    Geometry
    Raverie
    SpirvHeaders
    SpirvTools
    SpirvCross
)
