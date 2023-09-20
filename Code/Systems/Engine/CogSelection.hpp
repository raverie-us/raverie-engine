// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

void AddLinks(Cog* object, MetaSelection* selection);
void RemoveLinks(Cog* object, MetaSelection* selection);

/// Is the parent or (recursively) the parent in the selection.
bool IsAncestorInSelection(Cog* cog, MetaSelection* selection);
/// Is the parent or (recursively) the parent in the input container.
bool IsAncestorPresent(const Cog* cog, const Array<CogId>& cogs);
bool IsAncestorPresent(Cog* cog, const Array<Handle>& metaObjects);

/// Fill in array with only objects that have no Ancestor in selection.
void FilterChildrenAndProtected(Array<Cog*>& cogs, MetaSelection* selection, Array<Cog*>* filteredCogs = nullptr);
/// Filter out cogs that have no ancestor within the given input container
void FilterChildrenAndProtected(const Array<CogId>& cogsIn, Array<Cog*>& cogsOut, Array<Cog*>* filteredCogs = nullptr);
void FilterChildrenAndProtected(const Array<Handle>& objectsIn,
                                Array<Handle>& objectsOut,
                                Array<Handle>* filteredObjects = nullptr);

} // namespace Raverie
