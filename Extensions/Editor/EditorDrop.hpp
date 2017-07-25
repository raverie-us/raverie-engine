///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorDrop.hpp
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///Drop a resource on a object
bool DropOnObject(MetaDropEvent* event, Cog* droppedOn);
bool EditorDrop(MetaDropEvent* event, Viewport* viewport, Space* space, Cog* droppedOn);
}
