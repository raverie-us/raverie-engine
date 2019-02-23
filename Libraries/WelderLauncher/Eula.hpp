// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

String GetEulaFilePath();

class EulaWindow : public Composite
{
public:
  /// Typedefs.
  typedef EulaWindow ZilchSelf;

  /// Constructor.
  EulaWindow(Composite* parent);

  /// Button event response.
  void OnAccept(Event*);
  void OnCancel(Event*);
};

} // namespace Zero
