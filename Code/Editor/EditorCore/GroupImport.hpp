// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

void RunGroupImport(ImportOptions& options);
void GroupImport();
void OpenGroupImport(Array<String>& files);
void LoadDroppedFiles(Array<HandleOfString>& files);

// GroupImportWindow
class GroupImportWindow : public Composite
{
public:
  typedef GroupImportWindow RaverieSelf;

  ImportOptions* mOptions;
  PropertyView* mPropertyView;
  TextButton* mImportButton;
  TextButton* mCancelButton;
  ListBox* mListBox;
  Composite* mParentWindow;
  StringSource mStrings;

  GroupImportWindow(Composite* parent, ImportOptions* options);
  float GetPropertyGridHeight();
  void OnOptionsModified(Event* event);
  void RebuildTree();
  void UpdateListBoxSource();
  void OnPressed(Event* event);
  void OnCancel(Event* event);
};

// ImportCallback
class ImportCallback : public SafeId32EventObject
{
public:
  typedef ImportCallback RaverieSelf;

  void Open();
  void OnFilesSelected(OsFileSelection* fileSelection);
};

} // namespace Raverie
