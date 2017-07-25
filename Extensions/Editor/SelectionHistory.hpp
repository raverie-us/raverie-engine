///////////////////////////////////////////////////////////////////////////////
///
/// \file SelectionHistory.hpp
/// 
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class MainPropertyView;

//------------------------------------------------------------ Selection History
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

  void OnRecent(Composite* parent);

  void MovedToObject(MetaSelection* selection);
};

}//namespace Zero
