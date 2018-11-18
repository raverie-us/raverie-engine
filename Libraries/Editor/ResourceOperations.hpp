///////////////////////////////////////////////////////////////////////////////
///
/// \file ResourceOperations.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Editor Resource management functions. These functions work with the  
// content system and the resource system to manage resources for the editor.

// Add a new resource all resource adding goes through this function
Resource* AddNewResource(ResourceManager* resourceManager, ResourceAdd& resourceAdd);

// Add a resource to the current project from a file
Resource* AddResourceFromFile(StringParam filePath, StringParam resourceType);

// Add a resources to the current project from  files
void AddResourcesFromFiles(const Array<String>& files, StringParam resourceType);

// Duplicate a resource
Resource* DuplicateResource(Resource* resource, StringParam newName = String());

// Rename a resource
bool RenameResource(Resource* resource, StringParam newName);

// Reload a resource and content item
void ReloadResource(Resource* resource);
void ReloadContentItem(ContentItem* contentItem);

// Edit the resource in an external program
void EditResourceExternal(Resource* resource);

// Remove the resource project the project
void RemoveResource(Resource* resource);

// Load a resource from a content item
Resource* LoadResourceFromNewContentItem(ResourceManager* resourceManager, ContentItem* newContentItem, Resource* resource);

// Get what the file name should be for this resource
String GetResourceFileName(ResourceManager* resourceManager, StringParam resourceName, ContentItem* current = NULL);

// Where to place deleted files
String GetEditorTrash();

Resource* NewResourceOnWrite(ResourceManager* resourceManager, BoundType* type, StringParam property, Space* space, Resource* resource, Archetype* archetype, bool modified);

}
