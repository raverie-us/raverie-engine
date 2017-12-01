///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ZilchSpirVFrontEnd;
class ZilchSpirVFrontEndContext;
class EntryPointInfo;

void ValidateEntryPoint(ZilchSpirVFrontEnd* translator, Zilch::GenericFunctionNode* node, ZilchSpirVFrontEndContext* context);
void ValidateBasicEntryPoint(ZilchSpirVFrontEnd* translator, Zilch::GenericFunctionNode* node, ZilchSpirVFrontEndContext* context);
void ValidateGeometryEntryPoint(ZilchSpirVFrontEnd* translator, Zilch::GenericFunctionNode* node, ZilchSpirVFrontEndContext* context);

}//namespace Zero
