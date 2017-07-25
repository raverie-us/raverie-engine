///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------- Discover Tile
class DiscoverTile : public Composite
{
public:
  /// Typedefs.
  typedef DiscoverTile ZilchSelf;

  /// Constructor.
  DiscoverTile(Composite* parent, StringParam text, StringParam fontStyle,
               float titleHeight, StringParam url);

  /// Composite Interface.
  void UpdateTransform() override;

  /// Event Response.
  void OnMouseEnter(MouseEvent* e);
  void OnLeftClick(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);

  /// The height of the title background.
  float mTitleBgHeight;

  /// The background behind the text.
  Element* mBackground;

  Text* mTitle;

  /// Used to make the entire tile interactive.
  Element* mInteractive;

  String mUrl;
};

//------------------------------------------------------------- Dev Update Entry
class DevUpdateEntry : public Composite
{
public:
  /// Constructor.
  DevUpdateEntry(Composite* parent, DeveloperNotes& notes);

  Text* mNewText;
  Text* mNewLabel;
  Text* mDate;
  MultiLineText* mUpdate;
};

//---------------------------------------------------------------- Discover Menu
class DiscoverMenu : public Composite
{
public:
  /// Typedefs.
  typedef DiscoverMenu ZilchSelf;

  /// Constructor.
  DiscoverMenu(Composite* parent, LauncherWindow* launcher);

  /// Composite Interface.
  void UpdateTransform() override;

private:
  void CreateTiles(Composite* parent);

  void DownloadDevNotes();
  void OnCheckForUpdates(Event* e);
  void OnDeveloperNotesDownloaded(BackgroundTaskEvent* e);

  /// We want to hide the launcher when the discover page is active.
  void OnMenuDisplayed(Event* e);

  Composite* mTiles;
  Element* mBackground;
  LauncherWindow* mLauncher;
  ScrollArea* mDevUpdatesArea;
  Array<DevUpdateEntry*> mDevUpdateEntries;
};

}//namespace Zero
