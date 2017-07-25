/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2014-2014, DigiPen Institute of Technology
\**************************************************************/

"use strict";

// Insert text into an actual dom element (not jquery object)
// This is meant to insert text into a text area
function insertTextAtCursor(element, text)
{
  // If the element is read only, don't allow editing it
  if (element.readOnly)
    return;
  
  // This determines which browser we're on, and caches a few variables
  var val = element.value, endIndex, range, doc = element.ownerDocument;
  if (typeof element.selectionStart == "number" && typeof element.selectionEnd == "number")
  {
    // Basically just append the text before the selection, then our text, and then the text after the selection
    // This effectively replaces the selection
    var startIndex = element.selectionStart;
    var endIndex = element.selectionEnd;
    element.value = val.slice(0, startIndex) + text + val.slice(endIndex);
    element.selectionStart = startIndex + text.length;
    element.selectionEnd = element.selectionStart;
  }
  else if (doc.selection != "undefined" && doc.selection.createRange)
  {
    // Other browsers have a more compact way of replacing selection text
    element.focus();
    var range = doc.selection.createRange();
    range.collapse(false);
    range.text = text;
    range.select();
  }
}

// Closes a grid and all descending/expanded children
// Also updates the parent to show an unexpanded row
function closeHoverGrid(gridDom)
{
  // If the element is no longer visible, don't do anything!
  if (!gridDom.is(':visible'))
    return;
  
  // If we have a parent grid... change its expanded cell to not be a +
  if (gridDom.parentGridDom)
  {
    // Since we exist, and we're being closed (and we're visible!)
    // then the parent grid must have an expanded cell
    // Make sure to set the expanded cell back to +, and clear out any expanded properties
    var parentGridDom = gridDom.parentGridDom;
    parentGridDom.expandedCellDom.html('+');
    parentGridDom.expandedCellDom = null;
    parentGridDom.expandedRow = null;
    parentGridDom.expandedRowIndex = null;
    
    // Enable scrolling on our parent again
    parentGridDom.css('overflow-y', 'scroll');
  }

  // A function to recursively close grids (as well as mark clear their parent's grid's pointer to themselves)
  function closeGridAndChild(gridDomIterator)
  {
    // If this grid has an expanded grid, recursively close that too
    if (gridDomIterator.expandedGridDom)
      closeGridAndChild(gridDomIterator.expandedGridDom);
    
    // Remove our grid from the dom and clear out the parent's pointer to us
    gridDomIterator.remove();
    if (gridDom.parentGridDom)
      gridDom.parentGridDom.expandedGridDom = null;
  };
  
  // Recursively close all child grids (and this grid itself)
  closeGridAndChild(gridDom);
}

// Fills a hover grid with content
// The columns is an array of objects with name properties
// The rows is an array of objects whose properties share the names of the columns
function fillHoverGrid(gridDom, columns, rows)
{
  // If the element is no longer visible, don't do anything!
  if (!gridDom.is(':visible'))
    return;
  
  // Clear the inner html, this clears any loading icons
  gridDom.html('');
  
  // Also clear the 'hovertree-loading' css class, only used for making sure the loading box was a certain height
  gridDom.removeClass('hovertree-loading');
  
  // Also (just for safety) clear out any expanded information
  // This only occurs if the user calls fill twice on a gridDom (they shouldn't, but I suppose it would work!)
  gridDom.expandedGridDom = null;
  gridDom.expandedCellDom = null;
  gridDom.expandedRow = null;
  gridDom.expandedRowIndex = null;
    
  // Compute the total height so we can set the max height of the window
  // (if it's less then the already defined max height)
  // This is basically to fix an issue with firefox and overflow scrolling
  var totalHeight = 0;
  
  // Loop through all the columns
  for (var c = -1; c < columns.length; ++c)
  {
    // The first column will be undefined, this is ok!
    var column = columns[c];
    
    // Create the column div and append it to the grid
    var columnDom = $('<div class="column" id="column' + c + '"></div>');
    gridDom.append(columnDom);
    
    // Loop through all the rows that the user provided
    for (var r = 0; r < rows.length; ++r)
    {
      // Grab the current row
      var row = rows[r];
      
      // We're going to create a cell for this row/column below
      var cellDom = null;
      
      // Give rows an even and odd style
      var cellStyle = 'cell-base row-even';
      if ((r % 2) == 1)
        cellStyle = 'cell-base row-odd';
      
      // Add the rightmost or leftmost styles to the cells
      if (c == -1)
        cellStyle += ' cell-left-most';
      else if (c == (columns.length - 1))
        cellStyle += ' cell-right-most';
      else
        cellStyle += ' cell-inner-horizontal';
      
      // Add the topmost or bottommost styles to the cells
      if (r == 0)
        cellStyle += ' cell-top-most';
      else if (r == (rows.length - 1))
        cellStyle += ' cell-bottom-most';
      else
        cellStyle += ' cell-inner-vertical';
      
      // If the column is '-1', a special column only for expanders
      if (c == -1)
      {
        // If this row is expandable (then we need an expander +)
        if (row.expandable)
        {
          // Create the current cell
          cellDom = $('<div class="' + cellStyle + ' expander">+</div>');
          (function(expanderCellDom, rowIndex, row)
          {
            // Upon clicking the expander (we could be expanding or collapsing)
            expanderCellDom.click(function(e)
            {
              // We don't want the main grid dom to recieve this click event (or anyone else!)
              e.stopPropagation();
              
              // Store the last expanded cell so that we know if we're clicking on our own cell
              // or another cell (if we're clicking on our own cell, we're really just collapsing it)
              var lastExpandedCellDom = gridDom.expandedCellDom;
              
              // If we have a child expanded grid, close that (also resets our expanded row to a +)
              if (gridDom.expandedGridDom)
                closeHoverGrid(gridDom.expandedGridDom);
              
              // As long as we're not clicking on our own expander (which would collapse)
              if (lastExpandedCellDom != expanderCellDom)
              {
                // Get the offset to the expander button
                var offset = expanderCellDom.offset();
                
                // Turn the expander button into a '-' sign
                expanderCellDom.html('-');
                
                // Create a new expanded grid and store it on ourselves (so we know our expanded child)
                gridDom.expandedGridDom = createHoverGrid(
                {
                  left: offset.left + gridDom.childLeftOffset,
                  top: offset.top + gridDom.childTopOffset,
                  parentGridDom: gridDom,
                  expanderCallback: gridDom.expanderCallback,
                  cellModifiedCallback: gridDom.cellModifiedCallback
                });
                
                // Also store the cell that was expanded, plus the row and row index
                gridDom.expandedCellDom = expanderCellDom;
                gridDom.expandedRow = row;
                gridDom.expandedRowIndex = rowIndex;
                
                // Disable scrolling on the parent of this new expanded grid
                // This is so the parent's expander doesn't change positions due to scrolling
                gridDom.css('overflow-y', 'hidden');
                
                // Finally, create a list of expanded rows by walking from the new child
                // all the way back to our root parent (we'll know the root because we'll hit null)
                var expandedRows = [];
                var expandedRowIndices = [];
                var parentGridDomIt = gridDom;
                while (parentGridDomIt)
                {
                  // Push each expanded row and its index into separate arrays
                  // The order will be backwards, but we'll reverse below
                  expandedRows.push(parentGridDomIt.expandedRow);
                  expandedRowIndices.push(parentGridDomIt.expandedRowIndex);
                  
                  // Iterate up to the next parent
                  parentGridDomIt = parentGridDomIt.parentGridDom;
                }
                
                // We have to reverse the arrays (because we build them from child up to parent)
                expandedRows.reverse();
                expandedRowIndices.reverse();
                
                // Let the user know we expanded a row
                // Typically the user will invoke some kind of ajax query to fill out the expanded
                // grid, or instantly fill it out using 'fillHoverGrid'
                if (gridDom.expanderCallback)
                  gridDom.expanderCallback(gridDom, gridDom.expandedGridDom, rowIndex, row, expandedRowIndices, expandedRows);
              }
            });
          })(cellDom, r, row); // Capture values locally for a closure
        }
        else
        {
          // Create a non-expandable cell (no + or -)
          cellDom = $('<div class="' + cellStyle + '"> </div>');
        }
      }
      else
      {
        // Create a normal cell for the user's data to go in
        cellDom = $('<div class="' + cellStyle + ' cell"></div>');
        
        // Index the row using the column's name and insert that as the text
        cellDom.text(row[column.name]);
        
        // We're not yet editing this cell (used below)
        cellDom.editing = false;
        
        // Capture the local state using a closure
        (function(cellDom, columnIndex, column, rowIndex, row)
        {
          // When we lose focus, or submit the text, we destroy the
          // editable text and return it back to standard html text
          function destroyEditableText(e)
          {
            // If we're not editing this element, then skip out early
            if (cellDom.editing == false)
              return;
              
            // Get the text inside the text area
            var textArea = cellDom.children('textarea');
            var newText = textArea.val();
            
            // Set the text of the cell back to the entered text in the text area, then clear the editing flag
            cellDom.html(newText);
            cellDom.editing = false;
            
            // Let the user know that a cell has been modified (if the text actually changed!)
            // They may later update the value, but this should be handled gracefully
            if (gridDom.cellModifiedCallback && cellDom.originalText != newText)
              gridDom.cellModifiedCallback(gridDom, columnIndex, column, rowIndex, row, newText);
            
            // Update the value in the row (it may change types to a string...)
            row[column.name] = newText;
          }
          
          // When the user attempts to edit a cell's text...
          function createEditableText(e)
          {
            // If we're already editing, just early out and skip this
            if (cellDom.editing)
              return;
            
            // Mark this cell as being edited
            cellDom.editing = true;
            
            // Get the text of this cell (so we can set that text in a new text area)
            var text = cellDom.text();
            
            // Store the original text so we can see if it was modified
            cellDom.originalText = text;
            
            // Create a text area, set its text, set it to match the cell's width
            var editableText = $('<textarea class="editable" />');
            editableText.val(text);
            editableText.width(cellDom.width());
            editableText.height(cellDom.height());
            
            // Clear the text in the cell normally, then append the text area
            cellDom.html('');
            cellDom.append(editableText);
            
            // Finally, focus on the text area so the user can start typing
            editableText.focus();
            
            // If the row's cell is NOT editable... mark it as read only
            if (!row[column.name + '_editable'])
              editableText.attr('readonly','readonly');
            
            // Select all the text in the editable text
            editableText.select();
            
            // Register an event that when we lose focus from the
            // text editor, we destroy it and return it back to a cell (also on remove!)
            editableText.focusout(destroyEditableText);
            
            // Upon key down on the editable text
            editableText.keydown(function (e)
            {
              // If the user presses enter and is holding control...
              // then insert a newline at the selection/cursor
              if (e.keyCode === 13 && e.ctrlKey)
                insertTextAtCursor(editableText.get(0), '\n');
            });
            
            // Upon key press (this is when the text box would normally handle the event itself, not keydown!)
            editableText.keypress(function(e)
            {
              // If the user presses enter and is NOT holding control
              if (e.keyCode === 13 && !e.ctrlKey)
              {
                // Return the text editor back to normal text (submit)
                destroyEditableText(e);
                
                // We handled the event and we don't want the text box to do anything
                return false;  
              }
            });
          }
          
          // When we click on this cell...
          cellDom.dblclick(createEditableText);
        })(cellDom, c, column, r, row);
      }
      
      // Last but not least, append the cell to the columns
      columnDom.append(cellDom);
      
      // If we're the expander column...
      // Total up the height of the expanders
      if (c == -1)
        totalHeight += cellDom.height();
    }
  }
  
  // To fix an issue in Firefox with overflow scrolling, we set
  // the max height of the grid dom ourselves (to the total of the cell heights)
  if (totalHeight < gridDom.css('max-height'))
    gridDom.css('max-height', totalHeight);
}

// Create the hover grid with a creationObject which must contain:
//  - left                  The left position in absolute coordinates
//  - top                   The top position in absolute coordinates
//  - childLeftOffset       How far from the expander we offset the child grid's left (default 10)
//  - childTopOffset        How far from the expander we offset the child grid's top (default 10)
//  - expanderCallback      (parentGridDom, gridDom, rowIndex, row, expandedRowIndices, expandedRows)
//  - cellModifiedCallback  (gridDom, columnIndex, column, rowIndex, row, value)
function createHoverGrid(creationObject)
{
  // Create a div for the grid, automatically show the loading icon
  var gridDom = $('<div class="hovertree hovertree-loading"><div class="loading-icon"></div></div>');
  
  // Offset the grid to the user's given x and y positions
  gridDom.css('left', creationObject.left);
  gridDom.css('top', creationObject.top);
  
  // Store the parent on the grid as well as any callbacks
  gridDom.parentGridDom = creationObject.parentGridDom;
  gridDom.expanderCallback = creationObject.expanderCallback;
  gridDom.cellModifiedCallback = creationObject.cellModifiedCallback;
  
  // If the top and left child offsets are given (otherwise default them to 10)
  gridDom.childLeftOffset = creationObject.childLeftOffset ? creationObject.childLeftOffset : 10;
  gridDom.childTopOffset = creationObject.childTopOffset ? creationObject.childTopOffset : 10;
  
  // Also clear out the expanded grid (because we have none) as well as cell and row
  gridDom.expandedGridDom = null;
  gridDom.expandedCellDom = null;
  gridDom.expandedRow = null;
  gridDom.expandedRowIndex = null;
  
  // If we click on the grid, then we want to close all child hover grids
  gridDom.click(function(e)
  {
    // Don't propegate this message any further (we handled it!)
    e.stopPropagation();
    
    // If we have a child, then close it (this will also mark the parent row expander as +)
    if (gridDom.expandedGridDom)
      closeHoverGrid(gridDom.expandedGridDom);
  });
  
  // If we mouse down anywhere on the document, then we want to close the grid
  $(document).mousedown(function(e)
  {
    // Only regarding left mouse down, close the grid
    if (e.which == 1)
    {
      // Change focus to something else (this will properly submit edited texts)
      $(document.activeElement).blur();
      
      // Close the hover grid (technically closes all hover grids because
      // this will run multiple times, as well as closing children)
      closeHoverGrid(gridDom);
    }
  });
  
  // If we mouse down on the grid, we don't want the above logic to run
  // The message would normally bubble from the grid all the way to the document...
  gridDom.mousedown(function(e)
  {
    // If the left mouse is down, stop propagation of the event to the document
    if (e.which == 1)
      e.stopPropagation();
  });
  
  // Append the grid to the body (it will be absolutely positioned at the user's x and y)
  $('body').append(gridDom);
  return gridDom;
}
