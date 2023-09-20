// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Loading Window
class LoadingWindow : public Composite
{
public:
  typedef LoadingWindow RaverieSelf;
  LoadingWindow(Composite* composite);

  void Activate(StringParam taskName);
  void Deactivate();
  void SetLoadingName(StringParam text);
  void OnUpdate(UpdateEvent* event);
  void UpdateTransform();

private:
  Text* mMainText;
  Text* mPendingText;
  float mTime;
  // Used to darken screen and block input
  HandleOf<ColorBlock> mDarkScreen;
};

} // namespace Raverie
