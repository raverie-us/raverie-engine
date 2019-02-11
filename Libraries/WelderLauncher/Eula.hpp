// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

String GetEulaFilePath(Cog* configCog);

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

} // namespace Zero
