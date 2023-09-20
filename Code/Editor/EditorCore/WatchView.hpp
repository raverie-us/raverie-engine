// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class TreeView;
class TreeEvent;
class WatchViewSource;
struct TreeFormatting;

class WatchView : public Composite
{
public:
  typedef SpriteFrame RaverieSelf;

  WatchView(Composite* parent);
  ~WatchView();

private:
  void BuildFormat(TreeFormatting& formatting);

  TreeView* mTree;
  WatchViewSource* mSource;
};

} // namespace Raverie
