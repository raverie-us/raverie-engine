#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(GraphicalSort);
}

ZilchDefineType(GraphicalEntry, builder, type)
{
  ZilchBindGetter(Cog);
  ZilchBindMethod(SetGraphicalSortValue);
}

bool GraphicalEntry::operator<(const GraphicalEntry& other) const
{
  return mSort < other.mSort;
}

Cog* GraphicalEntry::GetCog()
{
  return mData->mGraphical->mOwner;
}

void GraphicalEntry::SetGraphicalSortValue(s32 sortValue)
{
  mSort &= 0xFFFFFFFF00000000;
  if (sortValue < 0)
    mSort |= (u64)~sortValue;
  else
    mSort |= (u64)(sortValue ^ 0x80000000);
}

void GraphicalEntry::SetRenderGroupSortValue(s32 sortValue)
{
  mSort &= 0x00000000FFFFFFFF;
  mSort |= (u64)sortValue << 32;
}

ZilchDefineType(GraphicalSortEvent, builder, type)
{
  ZilchBindGetterProperty(GraphicalEntries);
  ZilchBindGetterProperty(RenderGroup);

  ZeroBindEvent(Events::GraphicalSort, GraphicalSortEvent);
}

s32 GetGraphicalSortValue(Graphical& graphical, GraphicalSortMethod::Enum sortMethod, Vec3 pos, Vec3 camPos, Vec3 camDir)
{
  s32 value = 0;
  float* floatValue = (float*)&value;

  switch (sortMethod)
  {
    case GraphicalSortMethod::GraphicalSortValue: value = graphical.mGroupSortValue; break;
    case GraphicalSortMethod::BackToFrontView: *floatValue = -Math::Dot(pos - camPos, camDir); break;
    case GraphicalSortMethod::FrontToBackView: *floatValue = Math::Dot(pos - camPos, camDir); break;
    case GraphicalSortMethod::NegativeToPositiveX: *floatValue = pos.x; break;
    case GraphicalSortMethod::PositiveToNegativeX: *floatValue = -pos.x; break;
    case GraphicalSortMethod::NegativeToPositiveY: *floatValue = pos.y; break;
    case GraphicalSortMethod::PositiveToNegativeY: *floatValue = -pos.y; break;
    case GraphicalSortMethod::NegativeToPositiveZ: *floatValue = pos.z; break;
    case GraphicalSortMethod::PositiveToNegativeZ: *floatValue = -pos.z; break;
  }

  return value;
}

} // namespace Zero
