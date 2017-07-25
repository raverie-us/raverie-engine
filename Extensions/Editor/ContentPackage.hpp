///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentPackage.hpp
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct ContentPackageEntry
{
  // Will this item be import or exported
  bool Active;
  // Name of the content file
  String File;
  // Size of the content item
  uint Size;
  bool Conflicted;
};

struct ContentPackageListing
{
  ContentPackageListing(){}
  ~ContentPackageListing();

  /// File or source folder.
  String Location;

  void AddEntry(ContentPackageEntry* entry);

  HashMap<String, ContentPackageEntry*> Entries;
  Array<ContentPackageEntry*> SortedEntries;

  /// Tags that will be added to the content items when imported.
  HashSet<String> Tags;
};

// Load the listing of files contained within a resource package file
void LoadContentPackageListing(ContentPackageListing& listing, StringParam sourcePackageFile);

// Export give content listing to the destination package file
void ExportContentPackageListing(ContentPackageListing& listing, StringParam destPackageFile);

// Build content pPackage listing from current loaded content library
void BuildContentPackageListingFromLibrary(ContentPackageListing& listing, ContentLibrary* library);

// Import a content package listing from a sourcePackageFile
void ImportContentPackageListing(ContentPackageListing& listing, ContentLibrary* library, StringParam sourcePackageFile);

}//namespace Zero
