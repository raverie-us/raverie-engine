///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentPackageWidget.hpp
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class OsFileSelection;
class TagEditor;
class TreeEvent;
struct ContentImporterSource;

//------------------------------------------------------------- Content Importer
class ContentImporter : public Composite
{
public:
  typedef ContentImporter ZilchSelf;

  /// Constructor / Destructor
  ContentImporter(Composite* parent);
  ~ContentImporter();

  /// Opens a window for the given content package file.
  static void OpenImportWindow(StringParam fileName);

  /// Loads a listing from the given file name.
  void LoadListingFromFile(StringParam filename);

private:
  /// Sets the formatting for all the columns on the tree view.
  void SetTreeFormatting();

  /// Event response.
  void OnImport(Event* e);
  void OnCancel(Event* e);
  void OnImportFileSelected(OsFileSelection* e);

  /// Row events for displaying tool tips when the mouse is
  /// over a conflicted content item entry.
  void OnMouseEnterTreeRow(TreeEvent* e);
  void OnMouseExitTreeRow(TreeEvent* e);

  /// The location of the content package.
  String mFileName;

  /// Preview of the items in the content package file.
  ContentPackageListing mListing;

  /// Displays all items in the content listing.
  TreeView* mTreeView;

  /// Allows for the addition of tags when importing the content items.
  TagEditor* mTagEditor;

  /// Handle to the tooltip that explains conflicted entries.
  HandleOf<Widget> mToolTip;

  ContentImporterSource* mSource;
};

void ImportContentPackage();

}//namespace Zero
