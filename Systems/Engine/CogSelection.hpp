///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

void AddLinks(Cog* object, MetaSelection* selection);
void RemoveLinks(Cog* object, MetaSelection* selection);


///Is the parent or (recursively) the parent in the selection.
bool IsAncestorInSelection(Cog* cog, MetaSelection* selection);
///Is the parent or (recursively) the parent in the input container.
bool IsAncestorPresent(const Cog* cog, const Array<CogId>& cogs);
bool IsAncestorPresent(Cog* cog, const Array<Handle>& metaObjects);

///Fill in array with only objects that have no Ancestor in selection.
void FilterChildrenAndProtected(Array<Cog*>& cogs, MetaSelection* selection);
///Filter out cogs that have no ancestor within the given input container
void FilterChildrenAndProtected(const Array<CogId>& cogsIn, Array<Cog*>& cogsOut);
void FilterChildrenAndProtected(const Array<Handle>& objectsIn, Array<Handle>& objectsOut);

}
