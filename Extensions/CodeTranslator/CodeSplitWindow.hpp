///////////////////////////////////////////////////////////////////////////////
///
/// \file CodeSplitWindow.hpp
///  Declare the CodeSplitWindow and CodeSplitWindow.
///
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class TextEditor;
class ScriptEditor;
class Splitter;
class KeyboardEvent;

/// Interface for the code translator.
struct CodeTranslator
{
  virtual ~CodeTranslator() {};
  virtual void Translate(HashMap<String, String>& files);

  virtual uint GetSourceLexer();
  virtual uint GetDestinationLexer();
};

/// Ui widget to display two code windows
class CodeSplitWindow : public Composite
{
public:
  typedef CodeSplitWindow ZilchSelf;
  CodeSplitWindow(Composite* parent);

  void OnKeyDown(KeyboardEvent* event);
  void SetLexers(CodeTranslator* translator);

  ScriptEditor* mSourceText;
  ScriptEditor* mTranslatedText;
  Splitter* mSplitter;

  HandleOf<DocumentResource> mSourceResource;
  String mCommandToRunOnSave;
};

}//namespace Zero
