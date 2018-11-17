///////////////////////////////////////////////////////////////////////////////
///
/// \file ResourcePropertyOperations.hpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Fills out the given hash set with all resources used by properties 
// on the given object. Ignores hidden properties.
void GetResourcesFromProperties(HandleParam object,
                                HashSet<Resource*>& resources);

}//namespace Zero
