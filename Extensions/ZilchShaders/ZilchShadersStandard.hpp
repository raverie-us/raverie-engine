///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Math/MathStandard.hpp"
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
#include "LibraryTranslationHelpers.hpp"
#include "ShaderIntrinsicTypes.hpp"
#include "ZilchTypeCollector.hpp"
#include "Compositor.hpp"
#include "SimpleZilchShaderGenerator.hpp"
#include "ZilchShaderSettingsLibrary.hpp"

namespace Zilch
{

ZilchDeclareStaticLibrary(ShaderIntrinsicsLibrary, ZilchNoNamespace, ZeroNoImportExport);

}//namespace Zilch
