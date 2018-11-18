///////////////////////////////////////////////////////////////////////////////
///
/// \file SimpleAnimation.cpp
/// Implementation of the Layout widget support class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void AnimateLayout(Array<LayoutResult>& Results, bool animate)
{
  //Apply all the new layout
  Array<LayoutResult>::range results = Results.All();
  while(!results.Empty())
  {
    LayoutResult& result = results.Front();
    const float minChange = Pixels(1);
    float distanceMoved = (result.PlacedWidget->GetTranslation() - result.Translation).Length();
    float sizeChanged = (result.PlacedWidget->GetSize() - result.Size).Length();
    if(animate &&  (distanceMoved > minChange || sizeChanged > minChange) )
    {
      AnimateTo(result.PlacedWidget, result.Translation, result.Size);
    }
    else
    {
      result.PlacedWidget->SetTranslationAndSize(result.Translation, result.Size);
      result.PlacedWidget->UpdateTransformExternal();
    }
    results.PopFront();
  }
}


}