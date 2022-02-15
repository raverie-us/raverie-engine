// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class ListRow;
class ListColumnHeader;

class ListView : public Composite
{
public:
  typedef ListView ZilchSelf;

  static const float cHeaderRowHeight;
  static const float cResizerWidth;

  ListView(Composite* parent);
  ~ListView();

  void UpdateTransform() override;
  bool TakeFocusOverride() override;

  void ClearAllRows();
  ListRow* FindRowByIndex(DataIndex& index);

  void SetFormat(TreeFormatting* format);
  void SetDataSource(DataSource* dataSource);

  /// Updates the transform of each column.
  void UpdateColumnTransforms();
  /// Creates / Updates the headers.
  void UpdateHeaders();
  /// Creates / Updates the separators.
  void UpdateSeparators();
  /// Rebuild Separators.
  void UpdateColumnSeparators();
  /// Calculate the new text widget sizes after updating columns and headers.
  void UpdateTextUI();
  /// After a view-rebuild calculate the new view size.
  void UpdateSize();
  /// Expands all UI of the last row on the y-axis by 1.  Used when the
  /// ListView's final size is odd on the y-axis.
  void RedoLastRowUI();

public:
  /// Formatting option to fit each column to the max-row's text size in that
  /// column.
  Array<bool> mFitToText;
  /// Headers for each column
  Array<ListColumnHeader*> mHeaders;
  HashMap<ListColumnHeader*, ColumnResizer*> mHeaderResizers;

  /// Separators for each column
  Array<Element*> mColumnSeparators;
  /// Format for the view.
  TreeFormatting mFormatting;
  /// Data for all rows in the view.
  DataSource* mDataSource;

  /// The root node of the tree.
  ListRow* mRoot;

  /// The individual heights for each row.
  Array<float> mRowHeights;

  ScrollArea* mArea;

  /// Visible rows
  Array<ListRow*> mRows;
  HashMap<u64, ListRow*> mRowMap;
};

class ListRow : public TreeBase
{
public:
  typedef ListRow ZilchSelf;
  typedef InListBaseLink<ListRow, TreeBase> RowList;

  static const float cRowColor[];

  ListRow(ListView* listView, ListRow* rowParent, DataEntry* entry);
  ~ListRow();

  /// Compositions Interface
  void OnDestroy() override;

  bool IsRoot();

  /// Widget interface
  void UpdateTransform() override;
  void UpdateColumnsTransform();

  void RecursiveDestroy();
  void DestroyChildren();
  void RebuildChildren();

  void Refresh();

  void UpdateBgColor(uint index);

  void Fill(Array<ListRow*>& rows);

public:
  DataIndex mIndex;

  ListView* mList;

  uint mVisibleRowIndex;

  Element* mSeparator;
  Element* mGraphicBackground;

  ListRow* mParentRow;

  RowList mChildren;
  Array<Label*> mColumnContent;
};

class ListColumnHeader : public Composite
{
public:
  typedef ColumnHeader ZilchSelf;

  static const float cHeaderColor;
  static const float cColumnSeparatorColor;

  ListColumnHeader(ListView* list, ColumnFormat* format);

  void SetText(StringParam name);

  void UpdateTransform();

public:
  Element* mBackground;
  Label* mLabel;
  ColumnFormat* mFormat;
  ListView* mList;
};

} // namespace Zero
