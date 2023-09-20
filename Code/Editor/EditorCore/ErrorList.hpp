// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class TreeView;
class TreeEvent;
class ErrorListSource;
struct TreeFormatting;

/// Error list displays all errors and data about the errors in a list
class ErrorList : public Composite
{
public:
  typedef ErrorList RaverieSelf;

  ErrorList(Composite* parent);
  ~ErrorList();

  void ClearErrors();
  void OnScriptError(Event* event);

  // Widget Interface
  void UpdateTransform() override;
  void OnDataActivated(DataEvent* event);

private:
  void BuildFormat(TreeFormatting& formatting);

  TreeView* mTree;
  ErrorListSource* mSource;
};

} // namespace Raverie
