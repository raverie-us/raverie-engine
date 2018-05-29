///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Given all of the scripts in the shader libraries and a type name, link the zilch scripts
// into an executable state, run them, and collect the available render target results.
void ComputeZilchRenderResults(SimpleZilchShaderGenerator& shaderGenerator, StringParam zilchTypeName, RenderResults& results);
