///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

String GetEulaFilePath(Cog* configCog);

//------------------------------------------------------------------------- Eula
class EulaWindow : public Composite
{
public:
  /// Typedefs.
  typedef EulaWindow ZilchSelf;

  /// Constructor.
  EulaWindow(Cog* configCog, Composite* parent);

  /// Button event response.
  void OnAccept(Event*);
  void OnCancel(Event*);
};

}//namespace Zero
