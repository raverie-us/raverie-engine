///////////////////////////////////////////////////////////////////////////////
///
/// \file ErrorList.hpp
/// 
/// 
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class TreeView;
class TreeEvent;
class ErrorListSource;
struct TreeFormatting;

/// Error list displays all errors and data about the errors in a list
class ErrorList : public Composite
{
public:
  typedef ErrorList ZilchSelf;
  
  ErrorList(Composite* parent);
  ~ErrorList();

  void ClearErrors();
  void OnScriptError(Event* event);

  //Widget Interface
  void UpdateTransform() override;
  void OnDataActivated(DataEvent* event);

private:
  void BuildFormat(TreeFormatting& formatting);

  TreeView* mTree;
  ErrorListSource* mSource;
};

}// Namespace Zero
