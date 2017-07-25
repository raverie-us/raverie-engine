///////////////////////////////////////////////////////////////////////////////
///
/// \file CodeSplitWindow.cpp
///  Implement the CodeSplitWindow and CodeSplitWindow.
///
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void CodeTranslator::Translate(HashMap<String, String>& files)
{
  
}

uint CodeTranslator::GetSourceLexer()
{
  return Lexer::Python;
}

uint CodeTranslator::GetDestinationLexer()
{
  return Lexer::Python;
}

CodeSplitWindow::CodeSplitWindow(Composite* parent)
  : Composite(parent)
{
  SetLayout(CreateRowLayout());
  mSourceText = new ScriptEditor(this);
  mSplitter = new Splitter(this);
  mTranslatedText = new ScriptEditor(this);

  mSourceText->SetSizing(SizeAxis::X, SizePolicy::Flex, 200);
  mTranslatedText->SetSizing(SizeAxis::X, SizePolicy::Flex, 200);
  
  mSplitter->SetSize(Pixels(10,10));

  ConnectThisTo(this,Events::KeyDown,OnKeyDown);
}

void CodeSplitWindow::OnKeyDown(KeyboardEvent* event)
{
  if(event->Key == 'S' && event->CtrlPressed)
  {
    mSourceResource->SetAndSaveData(mSourceText->GetAllText());

    CommandManager* commands = CommandManager::GetInstance();
    
    Command* command = commands->GetCommand(mCommandToRunOnSave);
    if(command)
      command->Execute();
  }
}

void CodeSplitWindow::SetLexers(CodeTranslator* translator)
{
  mSourceText->SetLexer(translator->GetSourceLexer());
  mTranslatedText->SetLexer(translator->GetDestinationLexer());
}

}//namespace Zero
