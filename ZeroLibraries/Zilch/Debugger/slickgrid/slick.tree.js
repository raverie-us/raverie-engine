/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2014-2014, DigiPen Institute of Technology
\**************************************************************/

"use strict";

function removeColumnHeader(slickGrid, parentJQuery)
{
  parentJQuery.find('.slick-header-columns').css("height","0px").css('border', '0px');
  parentJQuery.find('.slick-header').removeClass('ui-state-default');
  slickGrid.resizeCanvas();
}

function initTreeGrid(slickGrid, expanderCallback)
{
  var dataView = slickGrid.getData();
  
  slickGrid.flatExpandedById = {};
  
  dataView.onRowCountChanged.subscribe(function(e, args)
  {
    slickGrid.updateRowCount();
    slickGrid.render();
  });
  
  dataView.onRowsChanged.subscribe(function(e, args)
  {
    slickGrid.invalidateRows(args.rows);
    slickGrid.render();
  });
  
  function treeFormatter(row, cell, value, columnDef, dataContext)
  {
    if (dataContext.loading)
    {
      return "<span class='slickgrid-loading-icon'></span>";
    }
    else
    {
      var parentCount = 0;
      
      var parent = dataContext.parent;
      while (parent)
      {
        ++parentCount;
        parent = parent.parent;
      }
      
      var spacer = "<span style='display:inline-block;height:1px;width:" + (15 * parentCount) + "px'></span>";
      var index = dataView.getIdxById(dataContext.id);
      
      // If the next row exists, and it's indent is greater then ours (then it means we have children)
      var data = dataView.getItems();
      var nextRowData = data[index + 1];
      if (nextRowData && nextRowData.parent == dataContext)
      {
        var expanded = slickGrid.flatExpandedById[dataContext.id];
        if (expanded)
        {
          return spacer + " <span class='slick-tree-toggle'>-</span>&nbsp;" + value;
        }
        else
        {
          return spacer + " <span class='slick-tree-toggle'>+</span>&nbsp;" + value;
        }
      }
      else
      {
        return spacer + " <span class='slick-tree-toggle'>&nbsp;</span>&nbsp;" + value;
      }
    }
  };
  
  // Apply the tree formatter to the first column that has 'tree'
  var columns = slickGrid.getColumns();
  var setTreeFormat = false;
  for (var i = 0; i < columns.length; ++i)
  {
    var column = columns[i];
    if (column.tree)
    {
      column.formatter = treeFormatter;
      setTreeFormat = true;
    }
  }
  
  if (setTreeFormat == false)
    columns[0].formatter = treeFormatter;
  
  slickGrid.onClick.subscribe(function(e, args)
  {
    if ($(e.target).hasClass('slick-tree-toggle'))
    {
      var item = dataView.getItem(args.row);
      if (item)
      {
        slickGrid.flatExpandedById[item.id] = !slickGrid.flatExpandedById[item.id];
        if (expanderCallback)
          expanderCallback(args.grid, item, args.row, slickGrid.flatExpandedById[item.id]);
        dataView.updateItem(item.id, item);
        slickGrid.invalidate();
      }
      e.stopImmediatePropagation();
    }
  });
  
  function treeFilter(item)
  {
    var parent = item.parent;
    
    while (parent)
    {
      if (!slickGrid.flatExpandedById[parent.id])
        return false;
      
      parent = parent.parent;
    }

    return true;
  };
  
  dataView.beginUpdate();
  dataView.setFilter(treeFilter);
  dataView.endUpdate();
  
  slickGrid.invalidate();
}
