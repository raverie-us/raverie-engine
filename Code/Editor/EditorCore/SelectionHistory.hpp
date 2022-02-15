// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class MainPropertyView;

class SelectionHistory : public EventObject
{
public:
  typedef SpriteFrame ZilchSelf;

  SelectionHistory();
  ~SelectionHistory();

  Array<MetaSelection*> mPrevious;
  Array<MetaSelection*> mNext;
  MetaSelection* mCurrent;
  bool mLocked;

  MetaSelection* Advance(HandleParam object);

  void Next();
  void Previous();
  void ShowObject();
  void Reselect();
  void Clear();

  void MovedToObject(MetaSelection* selection);
};

} // namespace Zero
