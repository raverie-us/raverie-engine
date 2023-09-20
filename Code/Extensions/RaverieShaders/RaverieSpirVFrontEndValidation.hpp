// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class RaverieSpirVFrontEnd;
class RaverieSpirVFrontEndContext;
class EntryPointInfo;

void ValidateEntryPoint(RaverieSpirVFrontEnd* translator, Raverie::GenericFunctionNode* node, RaverieSpirVFrontEndContext* context);
void ValidateBasicEntryPoint(RaverieSpirVFrontEnd* translator, Raverie::GenericFunctionNode* node, RaverieSpirVFrontEndContext* context);
void ValidateGeometryEntryPoint(RaverieSpirVFrontEnd* translator, Raverie::GenericFunctionNode* node, RaverieSpirVFrontEndContext* context);

} // namespace Raverie
