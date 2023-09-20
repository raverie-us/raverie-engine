// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Drop a resource on a object
bool DropOnObject(MetaDropEvent* event, Cog* droppedOn);
bool EditorDrop(MetaDropEvent* event, Viewport* viewport, Space* space, Cog* droppedOn);
} // namespace Raverie
