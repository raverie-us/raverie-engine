/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  DocumentationProperty::DocumentationProperty() :
    IsSettable(true),
    IsGettable(true),
    IsField(true)
  {
  }

  //***************************************************************************
  DocumentationType::DocumentationType() :
    IsValueType(false)
  {
  }

  //***************************************************************************
  RstTable::RstTable() :
    Columns(0),
    Rows(0),
    HeaderRows(0)
  {
  }

  //***************************************************************************
  void RstTable::Resize(size_t columns, size_t rows)
  {
    this->Cells.Resize(columns * rows);
    this->Columns = columns;
    this->Rows = rows;
  }

  //***************************************************************************
  void RstTable::SetCell(StringParam value, size_t column, size_t row)
  {
    this->Cells[column + row * this->Columns] = value;
  }

  //***************************************************************************
  String RstTable::GetCell(size_t column, size_t row)
  {
    return this->Cells[column + row * this->Columns];
  }

  //***************************************************************************
  void RstBuilder::WriteLine(RstTable& table)
  {
    if (table.Columns == 0 || table.Rows == 0)
      return;

    // We need to get the max size of all of the entries in each column
    Array<size_t> columnLengths;
    columnLengths.Resize(table.Columns);

    // Get the lengths of each of the columns
    for (size_t c = 0; c < table.Columns; ++c)
    {
      size_t columnLength = 0;

      for (size_t r = 0; r < table.Rows; ++r)
      {
        // Get the max of the running column lengths
        size_t currentLength = table.GetCell(c, r).ComputeRuneCount();
        columnLength = Math::Max(columnLength, currentLength);
      }

      columnLengths[c] = columnLength;
    }

    // We include two padding spaces
    const size_t Padding = 2;

    // Walk through all the rows and print out their values
    // Note: We pretend we have one more row than we do, but we only do that to write out the last line bar
    for (size_t r = 0; r <= table.Rows; ++r)
    {
      // Output the top bar for the current row
      for (size_t c = 0; c < table.Columns; ++c)
      {
        this->Write("+");
        size_t columnLength = columnLengths[c];

        char rowSeparator = '-';
        if (r == table.HeaderRows)
          rowSeparator = '=';

        this->Write(String::Repeat(rowSeparator, columnLength + Padding));
      }
      this->WriteLine("+");

      // Don't write out cell values for the last bar
      if (r == table.Rows)
        break;

      // Now write out the cell values (mind the space, must match padding!)
      for (size_t c = 0; c < table.Columns; ++c)
      {
        this->Write("| ");
        size_t columnLength = columnLengths[c];
        String cell = table.GetCell(c, r);
        this->Write(cell);
        this->Write(String::Repeat(' ', columnLength - cell.ComputeRuneCount()));

        // Write out the ending padding space
        this->Write(" ");
      }
      this->WriteLine("|");
    }
  }

  //***************************************************************************
  void RstBuilder::WriteLineHeading(StringRange heading, RstHeadingType::Enum type)
  {
    char lineCharacter = ' ';
    bool needsOverline = false;

    switch (type)
    {
    case RstHeadingType::Part:
      lineCharacter = '#';
      needsOverline = true;
      break;
    case RstHeadingType::Chapter:
      lineCharacter = '=';
      needsOverline = true;
      break;
    case RstHeadingType::Section:
      lineCharacter = '=';
      break;
    case RstHeadingType::SubSection:
      lineCharacter = '-';
      break;
    case RstHeadingType::SubSubSection:
      lineCharacter = '^';
      break;
    case RstHeadingType::Paragraph:
      lineCharacter = '"';
      break;
    }

    // If we need an overline
    if (needsOverline)
      this->WriteLine(String::Repeat(lineCharacter, heading.ComputeRuneCount()));

    this->WriteLine(heading);
    this->WriteLine(String::Repeat(lineCharacter, heading.ComputeRuneCount()));
  }
}