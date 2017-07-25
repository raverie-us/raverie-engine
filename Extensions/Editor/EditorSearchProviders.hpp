///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorSearchProviders.hpp
/// Support for searching
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ResourceLibrary;
class PropertyInterface;

SearchProvider* GetObjectSearchProvider();
SearchProvider* GetFactoryProvider(HandleParam object, HandleOf<MetaComposition>& composition);
SearchProvider* GetResourceSearchProvider(ResourceLibrary* resourceLibrary = NULL, bool showHidden = false);
SearchProvider* GetResourceSearchProvider(HandleParam object);
void AddEditorProviders(SearchData& search);

}//namespace Zero
