/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  Composition::Composition()
  {
  }
  
  //***************************************************************************
  Composition::~Composition()
  {
  }

  //***************************************************************************
  Composition* Composition::GetBaseComposition()
  {
    return nullptr;
  }

  //***************************************************************************
  void Composition::ClearComponents()
  {
    this->Components.Clear();
  }
}
