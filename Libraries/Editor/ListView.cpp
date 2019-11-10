// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

const float ListView::cHeaderRowHeight = Pixels(20.0f);
const float ListView::cResizerWidth = Pixels(10.0f);

ListView::ListView(Composite* parent) : Composite(parent)
{
  mArea = new ScrollArea(this);
  mArea->SetClientSize(Pixels(20, 20));
  mMinSize = Vec2(100, 100);

  mDataSource = NULL;

  mRoot = NULL;
}

ListView::~ListView()
{
}

void ListView::UpdateTransform()
{
  // Build the column starting / ending positions.
  UpdateColumnTransforms();
  // Builder the column headers.
  UpdateHeaders();
  // Build and place the separators.
  UpdateSeparators();

  // Update all UI based on the rebuild above.
  UpdateTextUI();
  UpdateSize();

  // No odd sizes allowed.
  if (((int)mSize.y) % 2 == 1)
    RedoLastRowUI();

  // Update the scroll area to be the same size as the view.
  mArea->SetSize(mSize);
  // No scrolling on the x-axis.
  mArea->DisableScrollBar(0);

  bool foundStart = false;
  uint visibleRows = 0;
  uint startVisible = 1;

  float totalRowHeights = 0.0f;
  float areaForItems = mSize.y - cHeaderRowHeight;
  float scrollOffset = -mArea->GetClientOffset().y;

  //  i = 1: skip the root
  uint activeRows = mRows.Size();
  for (uint i = 1; i < activeRows; ++i)
  {
    float height = mRows[i]->mSize.y;

    // Find the first visible row...
    if (!foundStart)
    {
      if (scrollOffset - height > 0.0f)
      {
        ++startVisible;
        scrollOffset -= height;
      }
      else
      {
        foundStart = true;
      }
    }

    // ...then start counting the visible rows.
    if (foundStart && areaForItems - height >= 0.0f)
    {
      ++visibleRows;
      areaForItems -= height;
    }

    totalRowHeights += height;
  }

  // If there is even 1 pixel left of visibility for another row, then count it.
  if (areaForItems >= 1.0f)
    ++visibleRows;

  // Out of bounds sanity check.
  startVisible = Math::Min(activeRows, startVisible);
  // Compute the end of the visible rows.
  uint endVisible = Math::Min((size_t)mRows.Size(), (size_t)(startVisible + visibleRows));

  // Update the client size with the size of all the rows
  mArea->SetClientSize(Vec2(mSize.x, totalRowHeights));

  // Set the y-transform of each row in the tree.
  float rowY = cHeaderRowHeight;

  // Deactivate all rows before the first visible, excluding the root.
  for (uint i = 1; i < startVisible; ++i)
  {
    ListRow* row = mRows[i];
    row->SetActive(false);
    rowY += row->mSize.y;
  }

  float rowWidth = mSize.x;
  if (mArea->IsScrollBarVisible(1))
    rowWidth -= mArea->GetScrollBarSize();

  // Activate and move all visible rows.
  for (uint i = startVisible; i < endVisible; ++i)
  {
    ListRow* row = mRows[i];
    float height = row->mSize.y;

    row->SetActive(true);
    row->SetTranslation(Vec3(0, rowY, 0));
    row->SetSize(Vec2(rowWidth, height));

    rowY += height;

    row->mGraphicBackground->MoveToBack();
    row->mGraphicBackground->SetTranslation(Vec3(0, 0, 0));
    row->mGraphicBackground->SetSize(Vec2(rowWidth, height));

    row->UpdateBgColor(i);
  }

  // Deactivate all rows from first non visible to the number of rows.
  for (uint i = endVisible; i < activeRows; ++i)
  {
    ListRow* row = mRows[i];
    row->SetTranslation(Vec3(0, rowY, 0));
    row->SetActive(false);

    rowY += row->mSize.y;
  }

  Composite::UpdateTransform();
}

bool ListView::TakeFocusOverride()
{
  mArea->GetClientWidget()->SoftTakeFocus();
  return true;
}

void ListView::ClearAllRows()
{
  if (mRoot)
  {
    mRoot->RecursiveDestroy();
    mRoot = NULL;
  }
}

ListRow* ListView::FindRowByIndex(DataIndex& index)
{
  return mRowMap.FindValue(index.Id, NULL);
}

void ListView::SetFormat(TreeFormatting* format)
{
  if (&mFormatting != format)
    mFormatting = *format;

  // The root shouldn't be collapsable or invisible.
  mFormatting.Flags.SetState(FormatFlags::ShowRoot, false);

  auto headers = mHeaders.All();
  while (!headers.Empty())
  {
    ListColumnHeader* header = headers.Front();
    headers.PopFront();
    header->Destroy();
  }
  auto resizers = mHeaderResizers.All();
  while (!headers.Empty())
  {
    ColumnResizer* resizer = resizers.Front().second;
    resizers.PopFront();
    resizer->Destroy();
  }

  mHeaders.Clear();
  mHeaderResizers.Clear();

  SetDataSource(mDataSource);
}

void ListView::SetDataSource(DataSource* dataSource)
{
  ClearAllRows();

  mDataSource = dataSource;

  // Build a new List.
  if (mDataSource)
  {
    DataEntry* rootEntry = mDataSource->GetRoot();
    uint rowCount = dataSource->ChildCount(rootEntry);

    mRowHeights.Clear();
    mRowHeights.Resize(rowCount, 0.0f);

    mRoot = new ListRow(this, NULL, rootEntry);
    mRoot->RebuildChildren();

    mFitToText.Resize(mRoot->mColumnContent.Size(), false);

    UpdateSize();
  }
}

void ListView::UpdateColumnTransforms()
{
  float unfixedWidth = mSize.x;
  if (mArea->IsScrollBarVisible(1))
    unfixedWidth -= mArea->GetScrollBarSize();

  float totalFlex = 0.0f;

  // Calculate room we after all fixed columns.
  int c = 0;
  forRange (ColumnFormat& column, mFormatting.Columns.All())
  {
    // If the column is made to fit the text, then count it as fixed.
    if (mFitToText[c])
      unfixedWidth -= column.CurrSize.x;
    // If it's fixed, subtract its size
    else if (column.ColumnType != ColumnType::Flex)
      unfixedWidth -= column.FixedSize.x;
    else // If it's flex, add to the total flex size
      totalFlex += column.FlexSize;

    // Column index.
    c++;
  }

  // Used to keep track of where each column should start.
  float currentX = 0.0f;

  // Walk through each column and set its size and translation.
  c = 0;
  forRange (ColumnFormat& column, mFormatting.Columns.All())
  {
    // The starting position doesn't rely on the column type
    column.StartX = currentX;

    // Don't really need to do anything in this case except block the other
    // cases from occuring.
    if (mFitToText[c])
    {
      // should already be the right width.
      column.CurrSize.x = column.CurrSize.x;
    }
    // If it's fixed, assign it its desired size
    else if (column.ColumnType != ColumnType::Flex)
    {
      column.CurrSize.x = Math::Max(column.MinWidth, column.FixedSize.x);
    }
    else
    {
      // If it's not fixed, calculate the size based off the total flex.
      float flex = column.FlexSize / totalFlex;
      float width = unfixedWidth * flex;

      // if(mFormatting.Flags.IsSet(FormatFlags::ShowSeparators))
      //  width += 4.0f;

      column.CurrSize.x = width;
    }

    // Add the size to move onto the next column
    currentX += column.CurrSize.x;

    // Column index.
    c++;
  }
}

void ListView::UpdateHeaders()
{
  // Do nothing if we're not showing headers
  if (!mFormatting.Flags.IsSet(FormatFlags::ShowHeaders))
    return;

  uint currHeader = 0;

  uint columnCount = mFormatting.Columns.Size();
  for (uint i = 0; i < columnCount; ++i)
  {
    ColumnFormat& column = mFormatting.Columns[i];

    // Do nothing if there's no header.
    if (column.HeaderName.Empty())
      continue;

    // Create a new header.
    if (currHeader >= mHeaders.Size())
      mHeaders.PushBack(new ListColumnHeader(this, &column));

    ListColumnHeader* header = mHeaders[currHeader];
    header->SetText(column.HeaderName);

    float startPos = column.StartX;
    float extraWidth = 0.0f;

    // Bump the text over a little if there are separators.
    if (mFormatting.Flags.IsSet(FormatFlags::ShowSeparators))
      header->mLabel->mPadding.Left = 4.0f;

    //  // Walk left and keep pushing the start of the header left to eat any
    //  // columns that don't have a header.
    //  //   - NOTE: May have to do something similar for eating right
    // for(int j = (int)i - 1; j >= 0; --j)
    //{
    //  ColumnFormat& testFormat = mFormatting.Columns[j];
    //  if(!testFormat.HeaderName.Empty( ))
    //    break;

    //  startPos -= testFormat.CurrSize.x;
    //  extraWidth += testFormat.CurrSize.x;
    //}

    header->SetTranslation(SnapToPixels(Vec3(startPos, 0, 0)));

    // Set header size to the final width.
    float finalWidth = column.CurrSize.x + extraWidth;
    header->SetSize(SnapToPixels(Vec2(finalWidth, cHeaderRowHeight)));

    // Update the column resizers.
    if (mFormatting.Flags.IsSet(FormatFlags::ColumnsResizable))
    {
      // Attempt to find the current resizer for this header.
      ColumnResizer* resizer = mHeaderResizers.FindValue(header, NULL);
      if (resizer == NULL)
      {
        if ((i + 1) < columnCount)
        {
          ColumnFormat* nextFormat = &mFormatting.Columns[i + 1];
          resizer = new ColumnResizer(this, &column, nextFormat);
          mHeaderResizers.Insert(header, resizer);
        }
      }

      // Place the resizer to the right side of this header.
      if (resizer)
      {
        Vec3 resizerPos(startPos + finalWidth - cResizerWidth * 0.5f, 0, 0);
        resizer->SetTranslation(SnapToPixels(resizerPos));
        resizer->SetSize(SnapToPixels(Vec2(cResizerWidth, cHeaderRowHeight)));
      }
    }

    ++currHeader;
  }

  // Move all the resizers to the front of the headers so that they get hit
  // first with ray casts.
  typedef Pair<ListColumnHeader*, ColumnResizer*> ResizerPair;
  forRange (ResizerPair& entry, mHeaderResizers.All())
  {
    ColumnResizer* resizer = entry.second;
    resizer->MoveToFront();
  }
}

void ListView::UpdateSeparators()
{
  if (!mFormatting.Flags.IsSet(FormatFlags::ShowSeparators))
    return;

  UpdateColumnSeparators();
}

void ListView::UpdateColumnSeparators()
{
  bool foundFlex = false;
  uint separatorCount = 0;

  uint columnCount = mFormatting.Columns.Size();
  for (uint i = 0; i < columnCount; ++i)
  {
    // Don't need a separator for the first columns
    if (i == 0)
      continue;

    ColumnFormat& column = mFormatting.Columns[i];

    // Create a new separator, if needed.
    if (separatorCount >= mColumnSeparators.Size())
    {
      Vec4 color(ListColumnHeader::cColumnSeparatorColor);
      color.w = 1.0f;

      Element* separator = CreateAttached<Element>(cWhiteSquare);
      separator->SetColor(color);

      mColumnSeparators.PushBack(separator);
    }

    // Set the separator size and position.
    Element* separator = mColumnSeparators[separatorCount];
    separator->SetTranslation(SnapToPixels(Vec3(column.StartX, 0, 0)));
    separator->SetSize(SnapToPixels(Vec2(Pixels(2), mSize.y)));

    ++separatorCount;
  }
}

void ListView::UpdateTextUI()
{
  int rowCount = mRows.Size();
  for (int r = 1; r < rowCount; ++r) // r = 1: skip root
  {
    float height = 0.0f;
    float rowSize = 0.0f;

    ListRow* row = mRows[r];

    // Computed the size and transform for each column based on its text.
    uint columns = mFormatting.Columns.Size();
    for (uint i = 0; i < columns; ++i)
    {
      ColumnFormat& format = mFormatting.Columns[i];
      Label* content = row->mColumnContent[i];

      float fitWidth = format.CurrSize.x;
      if (format.ColumnType == ColumnType::Flex)
        fitWidth = RemoveThicknessRect(content->mPadding, Vec2(fitWidth, 0)).SizeX;

      // Allow ample height for text.
      Vec2 size(Pixels(fitWidth), Pixels(1000));
      // Measures the text's render size and sets it internally.
      content->mText->FitToWidth(size.x, size.y);

      // Account for separators and padding.
      if (mFormatting.Flags.IsSet(FormatFlags::ShowSeparators))
      {
        content->mText->mSize.x += 6.0f;
        content->mText->mSize.y += 6.0f;
      }
      else
      {
        content->mText->mSize.x += 4.0f;
        content->mText->mSize.y += 4.0f;
      }

      // Put a little more padding on right.
      content->mText->mSize.x += 12.0f;

      // Update to the measured text size.
      size = content->mText->mSize;
      content->SetSize(size);

      // Record the height for the entire row.
      height = Math::Max(height, size.y);
      // Update row width
      rowSize += format.CurrSize.x;
    }

    //  Make each column of this row a uniform height.
    mRowHeights[r - 1] = height;
    for (uint i = 0; i < columns; ++i)
    {
      Label* content = row->mColumnContent[i];
      content->SetSize(Vec2(content->mSize.x, height));
      // Place the text based on final size.
      content->UpdateTransform();
    }

    // Update the row UI.
    row->SetMinSize(Vec2(rowSize, height));
    row->SetSize(row->GetMinSize());
    row->UpdateTransform();
  }
}

void ListView::UpdateSize()
{
  mSize.Set(0, cHeaderRowHeight);

  // Compute the view height based on all rows.
  uint rowCount = mRowHeights.Size();
  for (uint i = 0; i < rowCount; ++i)
  {
    mSize.y += mRowHeights[i];
    mSize.x = Math::Max(mSize.x, mRows[i + 1]->mSize.x); // i+1: skip the root
  }

  mMinSize = mSize;
}

void ListView::RedoLastRowUI()
{
  ListRow* row = mRows.Back();

  float& height = mRowHeights.Back();
  height += 1.0f;

  //  Make each column of this row a uniform height.
  uint columns = mFormatting.Columns.Size();
  for (uint i = 0; i < columns; ++i)
  {
    Label* content = row->mColumnContent[i];
    content->SetSize(Vec2(content->mSize.x, height));
    // Place the text based on final size.
    content->UpdateTransform();

    // Feed back the final size to the formatting.
    mFormatting.Columns[i].CurrSize.y += 1.0f;
  }

  // Update the row UI.
  row->SetMinSize(Vec2(row->GetMinSize().x, height));
  row->SetSize(row->GetMinSize());
  // Update the row separator.
  if (mFormatting.Flags.IsSet(FormatFlags::ShowSeparators))
    row->mSeparator->SetTranslation(Vec3(0, mSize.y - Pixels(1), 0));

  // Update the ListView itself.
  mMinSize.y = mSize.y += 1.0f;
}

const float ListRow::cRowColor[] = {61.0f / 255.0f, 72.0f / 255.0f};

ListRow::ListRow(ListView* listView, ListRow* rowParent, DataEntry* entry) : TreeBase(listView->mArea)
{
  mList = listView;
  mActive = true;
  mIndex = listView->mDataSource->ToIndex(entry);

  mParentRow = rowParent;

  listView->mRowMap[mIndex.Id] = this;
  listView->mRows.PushBack(this);

  mGraphicBackground = CreateAttached<Element>(cWhiteSquare);
  mGraphicBackground->SetInteractive(false);

  TreeFormatting& format = mList->mFormatting;

  if (format.Flags.IsSet(FormatFlags::ShowSeparators))
  {
    mSeparator = CreateAttached<Element>("HorizontalDivider");
    mSeparator->SetColor(Vec4(1, 1, 1, 0.7f));
  }

  // Build Columns
  uint columns = format.Columns.Size();
  mColumnContent.Resize(columns);

  for (uint i = 0; i < columns; ++i)
  {
    Label* content = mColumnContent[i] = new Label(this, cText);
    content->SetColor(Vec4(1, 1, 1, 0.85f));
    content->mText->SetMultiLine(true);
    content->mText->mClipText = false;
    content->mText->MoveToFront();

    // Bump the text over a little if there are separators.
    if (format.Flags.IsSet(FormatFlags::ShowSeparators))
      content->mPadding.Left = 4.0f;
  }

  Refresh();

  // Background should be behind every other ui element.
  mGraphicBackground->MoveToBack();
}

ListRow::~ListRow()
{
}

void ListRow::OnDestroy()
{
  Composite::OnDestroy();
}

bool ListRow::IsRoot()
{
  DataEntry* root = mList->mDataSource->GetRoot();
  DataIndex rootIndex = mList->mDataSource->ToIndex(root);
  return (rootIndex == mIndex);
}

void ListRow::UpdateTransform()
{
  if (mActive == false)
    return;

  if (mList->mFormatting.Flags.IsSet(FormatFlags::ShowSeparators))
  {
    mSeparator->SetTranslation(Vec3(0, mSize.y - Pixels(1), 0));
    mSeparator->SetSize(Vec2(mSize.x, Pixels(2)));
  }

  // Update the column sizes and locations.
  UpdateColumnsTransform();

  Composite::UpdateTransform();
}

void ListRow::UpdateColumnsTransform()
{
  TreeFormatting& format = mList->mFormatting;

  // Walk through each column and set its size and translation.
  uint columnCount = format.Columns.Size();
  for (uint i = 0; i < columnCount; ++i)
  {
    ColumnFormat& column = format.Columns[i];
    Vec2 size(column.CurrSize.x, mSize.y);

    Label* content = mColumnContent[i];
    content->SetTranslation(SnapToPixels(Vec3(column.StartX, 0, 0)));
    content->SetSize(SnapToPixels(size));
  }
}

void ListRow::RecursiveDestroy()
{
  DestroyChildren();
  this->Destroy();
}

void ListRow::DestroyChildren()
{
  forRange (ListRow& child, mChildren.All())
  {
    child.RecursiveDestroy();
  }
}

/// Rebuild children by reusing valid rows and destroying invalid rows.
void ListRow::RebuildChildren()
{
  DataEntry* entry = mList->mDataSource->ToEntry(mIndex);

  if (entry == NULL)
    return;

  // Swap out old children.
  RowList oldChildren;
  oldChildren.Swap(mChildren);

  DataEntry* prevEntry = NULL;

  uint childCount = mList->mDataSource->ChildCount(entry);
  for (uint i = 0; i < childCount; ++i)
  {
    DataEntry* childEntry = mList->mDataSource->GetChild(entry, i, prevEntry);
    DataIndex index = mList->mDataSource->ToIndex(childEntry);

    // Is the row still valid and is 'this' still the row's parent?
    ListRow* row = mList->FindRowByIndex(index);
    if (row && row->mParentRow == this)
    {
      // Then, move to current children list.
      oldChildren.Erase(row);
      mChildren.PushBack(row);

      row->Refresh();
    }
    else
    {
      // No row exists at current data 'index', so create a new row.
      ListRow* treeRow = new ListRow(mList, this, childEntry);
      mChildren.PushBack(treeRow);
    }

    prevEntry = childEntry;
  }

  // Destroy all children that are no longer valid.
  while (!oldChildren.Empty())
  {
    ListRow* current = &oldChildren.Front();
    current->RecursiveDestroy();
    oldChildren.Erase(current);

    // Child removes itself in destruction.
    mChildren.PushBack(current);
  }

  this->MarkAsNeedsUpdate();
}

/// Load data from data source and size UI accordingly.
void ListRow::Refresh()
{
  // No need to refresh the data if this row is the root.
  if (IsRoot())
    return;

  TreeFormatting& formatting = mList->mFormatting;

  // Get data from source and update the column content.
  if (DataEntry* entry = mList->mDataSource->ToEntry(mIndex))
  {
    float offsetX = 0.0f;
    float height = 0.0f;

    // Computed the size and transform for each column based on its text.
    uint columns = formatting.Columns.Size();
    for (uint i = 0; i < columns; ++i)
    {
      Any value;
      ColumnFormat& format = formatting.Columns[i];
      mList->mDataSource->GetData(entry, value, format.Name);

      String final;
      String text = value.ToString();
      StringRange sRange = text.FindFirstOf("\\");
      while (!sRange.Empty())
      {
        StringSplitRange sSplit = text.Split(sRange);
        final = String::Join("", final, sSplit.mCurrentRange, "\n");
        text = sSplit.mRemainingRange;
        sRange = text.FindFirstOf("\\");
      }

      final = String::Join("", final, text);

      Label* content = mColumnContent[i];
      content->SetText(final);
      content->mPadding.Right += 6.0f;

      float fitWidth = format.MinWidth;
      if (format.ColumnType == ColumnType::Flex)
        fitWidth = RemoveThicknessRect(content->mPadding, Vec2(fitWidth, 0)).SizeX;

      // Allow ample height for text.
      Vec2 size(Pixels(fitWidth), Pixels(1000));
      // Measures the text's render size and sets it internally.
      content->mText->FitToWidth(size.x, size.y);

      // Account for separators and padding.
      if (formatting.Flags.IsSet(FormatFlags::ShowSeparators))
      {
        content->mText->mSize.x += 6.0f;
        content->mText->mSize.y += 6.0f;
      }
      else
      {
        content->mText->mSize.x += 4.0f;
        content->mText->mSize.y += 4.0f;
      }

      // Put a little more padding on right.
      content->mText->mSize.x += 12.0f;

      // Update to the measured text size.
      size = content->mText->mSize;
      content->SetSize(size);

      // Record the height for the entire row.
      height = Math::Max(height, size.y);

      // Feed back the computed values to the column format.
      format.StartX = offsetX;
      format.CurrSize.x = Math::Max(format.CurrSize.x, size.x);

      // Final transform.
      content->SetTranslation(Vec3(offsetX, 0, 0));
      offsetX += size.x;
    }

    //  Make each column of this row a uniform height.
    mList->mRowHeights[(uint)mIndex.Id] = height;
    for (uint i = 0; i < columns; ++i)
    {
      Label* content = mColumnContent[i];
      content->SetSize(Vec2(content->mSize.x, height));
      // Place the text based on final size.
      content->UpdateTransform();

      // Commented out because the y-axis isn't reset to 0.0f on ListView
      // rebuild.
      //  // Feed back the final size to the formatting.
      // formatting.Columns[i].CurrSize.y += height;
    }

    // Update the row UI.
    mMinSize = Vec2(offsetX, height);
    SetSize(mMinSize);
    UpdateTransform();
  }
  else
  {
    ErrorIf(true, "Lost Entry");
  }
}

void ListRow::UpdateBgColor(uint index)
{
  index %= 2;

  Vec4 color(cRowColor[index], cRowColor[index], cRowColor[index], 1.0f);
  mGraphicBackground->SetColor(color);
}

void ListRow::Fill(Array<ListRow*>& rows)
{
  if (!mActive || mDestroyed)
    return;

  rows.PushBack(this);
  this->mVisibleRowIndex = rows.Size() - 1;

  forRange (ListRow& child, mChildren.All())
  {
    child.Fill(rows);
  }
}

const float ListColumnHeader::cHeaderColor = 55.0f / 255.0f;
const float ListColumnHeader::cColumnSeparatorColor = 47.0f / 255.0f;

ListColumnHeader::ListColumnHeader(ListView* list, ColumnFormat* format) : Composite(list), mFormat(format), mList(list)
{
  Vec4 color = FloatColorRGBA(28, 60, 80, 255); //(cHeaderColor, cHeaderColor, cHeaderColor, 1.0f);

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(color);
  mLabel = NULL;
}

void ListColumnHeader::SetText(StringParam name)
{
  if (mLabel == NULL)
    mLabel = new Label(this, "BoldText");

  mLabel->SetText(name);
}

void ListColumnHeader::UpdateTransform()
{
  mBackground->SetSize(GetSize());

  if (mLabel)
  {
    mLabel->SetTranslation(Pixels(0, 0, 0));
    mLabel->SizeToContents();
    mLabel->SetColor(Vec4(1, 1, 1, 0.45f));
  }

  Composite::UpdateTransform();
}

} // namespace Zero
