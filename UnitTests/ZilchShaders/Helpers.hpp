///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Simple function to display a diff error in notepad++ (ideally use a better text editor or something later)
void DisplayError(StringParam filePath, StringParam lineNumber, StringParam errMsg);

void TestCompilation(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, ShaderType* type, StringParam directory, ErrorReporter& report);
void TestShaderFileCompilation(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, StringParam filePath, StringParam directory, ErrorReporter& report);
void TestShaderCompilationOfDirectory(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, StringParam directory, ErrorReporter& report);
void TestCompilationAndLinkingOfCompositesInDirectory(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, StringParam directory, ErrorReporter& report);
void TestRuntime(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, StringParam directory, ErrorReporter& report);

FragmentInfo BuildShaderAndDiff(SimpleZilchShaderGenerator& shaderGenerator, ZilchShaderDefinition& shaderDef, BaseRenderer& renderer, FragmentType::Enum fragmentType, StringParam directory, ErrorReporter& report);
