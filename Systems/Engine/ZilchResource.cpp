////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------------------------- Zilch Library Resource
//**************************************************************************************************
ZilchDefineType(ZilchLibraryResource, builder, type)
{
  ZeroBindDocumented();
}

//----------------------------------------------------------------------------------- Zilch Document
//**************************************************************************************************
ZilchDefineType(ZilchDocumentResource, builder, type)
{
  ZilchBindFieldGetter(mText);
  ZeroBindDocumented();
}

//**************************************************************************************************
ZilchDocumentResource::ZilchDocumentResource()
{
}

//**************************************************************************************************
StringRange ZilchDocumentResource::LoadTextData()
{
  return mText;
}

//**************************************************************************************************
void ZilchDocumentResource::ReloadData(StringRange data)
{
  mText = data;
}

//**************************************************************************************************
String ZilchDocumentResource::GetFormat()
{
  static String sFormat("Zilch");
  return sFormat;
}

//**************************************************************************************************
void ZilchDocumentResource::OnCharacterAdded(ICodeEditor* editor, Rune value)
{
  IndentOnNewlineWhenLastCharacterIs(editor, value, '{');

  String currentLine = editor->GetLineText(editor->GetCaretLine());
  if (currentLine.Contains("{") == false)
    UnindentOnCharacter(editor, value, '}');

  if (value == ')')
  {
    editor->HideCallTips();
  }

  // Are we accessing a member (function/property/data, either instance or static?)
  if (value == '.')
  {
    AutoCompleteInfo info;
    GetAutoCompleteInfo(editor, info);

    Array<Completion> completions;

    if (info.IsLiteral && info.NearestType == ZilchTypeId(int))
      return;

    forRange(CompletionEntry& entry, info.CompletionEntries.All())
    {
      Completion& completion = completions.PushBack();
      completion.Description = entry.Description;
      completion.Name = entry.Name;
      completion.SignaturePathType = entry.Type;
      completion.AssociatedResourceId = entry.CodeUserDataU64;
      completion.Hidden = entry.Hidden;
    }

    editor->ShowAutoComplete(completions, CompletionConfidence::Perfect);
  }
  else if (value == ':' || (value == ' ' && currentLine.FindLastNonWhitespaceRune() == ':'))
  {
    Array<Completion> completions;
    // The any keyword is special. Just force add it to our possible completion list.
    completions.PushBack(Completion("any"));

    Array<LibraryRef> libraries;
    GetLibraries(libraries);
    for (uint i = 0; i < libraries.Size(); ++i)
      AddTypesToCompletion(completions, libraries[i]);

    editor->ShowAutoComplete(completions, CompletionConfidence::Perfect);
  }
  // Are we attempting to call a function? (this may just be a grouping operator)
  else if (value == '(' || value == ',')
  {
    AutoCompleteInfo info;
    GetAutoCompleteInfo(editor, info);

    if (info.FunctionName.Empty() == false)
    {
      Array<CallTip> tips;
      forRange(CompletionOverload& overload, info.CompletionOverloads.All())
      {
        CallTip& tip = tips.PushBack();
        tip.Description = overload.Description;
        tip.Return = overload.ReturnType;

        forRange(CompletionParameter& param, overload.Parameters.All())
        {
          ParameterTip& paramTip = tip.Parameters.PushBack();
          paramTip.Description = param.Description;
          paramTip.Name = param.Name;
          paramTip.Type = param.Type;
        }
      }
      
      editor->ShowCallTips(tips, info.FunctionName, info.CallArgumentIndex);
    }
  }
}

//**************************************************************************************************
void ZilchDocumentResource::OnShowAutoComplete(ICodeEditor* editor)
{
  // We need to look at the previous character to see what to do
}

//**************************************************************************************************
String ZilchDocumentResource::GetSingleLineCommentToken()
{
  static const String Comment("//");
  return Comment;
}

//**************************************************************************************************
void AddTypesToAutoComplete(LibraryRef library, Array<Completion>& keywordsOut)
{
  if (library != nullptr)
  {
    forRange(BoundType* type, library->BoundTypes.Values())
    {
      keywordsOut.PushBack(Completion(type->Name, type->Description));
    }
    forRange(InstantiateTemplateInfo& templateHandler, library->TemplateHandlers.Values())
    {
      // Build the full name of the template
      StringBuilder nameBuilder;
      nameBuilder.Append(templateHandler.TemplateBaseName);
      nameBuilder.Append("[");
      size_t count = templateHandler.TemplateParameters.Size();
      for(size_t i = 0; i < count; ++i)
      {
        nameBuilder.Append(templateHandler.TemplateParameters[i].Name);
        if(i != count - 1)
          nameBuilder.Append(", ");
      }
      nameBuilder.Append("]");

      // Since auto-completion adds the full name, we can't put the default template argument names in there.
      // For now put the full name in the description so the user can at least see the required arguments.
      keywordsOut.PushBack(Completion(templateHandler.TemplateBaseName, nameBuilder.ToString()));
    }
  }
}

//**************************************************************************************************
void ZilchDocumentResource::GetKeywords(Array<Completion>& keywordsOut)
{
  // Add all types from the Zilch core
  AddTypesToAutoComplete(Core::GetInstance().GetLibrary(), keywordsOut);

  // Add all types from the zero binding
  Array<LibraryRef> libraries;
  GetLibraries(libraries);
  for (uint i = 0; i < libraries.Size(); ++i)
    AddTypesToAutoComplete(libraries[i], keywordsOut);
}

//**************************************************************************************************
bool ZilchDocumentResource::SupportsZeroConnect()
{
  return true;
}

//**************************************************************************************************
String ZilchDocumentResource::GenerateConnectCallEnd(StringParam functionName)
{
  return BuildString("this.", functionName, ");");
}

//**************************************************************************************************
String ZilchDocumentResource::GenerateConnectFunctionStart(StringParam functionName, StringParam eventType)
{
  StringBuilder builder;
  builder.Append("\n\n\ffunction ");
  builder.Append(functionName);
  builder.Append("(event : ");
  builder.Append(eventType);
  builder.Append(")\n\f{\n\f\t");
  return builder.ToString();
}

//**************************************************************************************************
String ZilchDocumentResource::GenerateConnectFunctionEnd()
{
  return "\n\f}";
}

//**************************************************************************************************
void ZilchDocumentResource::FindPositionToGenerateFunction(ICodeEditor* editor, int& positionOut, String& indent)
{
  // Assume we can't find it, until we do!
  positionOut = -1;

  String allText = editor->GetAllText();
  size_t caret = editor->GetCaretPosition();

  StringIterator start = allText.Begin();
  StringIterator end = start + caret;

  // Check to make sure this is being connected inside a function
  StringRange function = allText.SubString(start, end).FindLastOf("function");
  if (function.Empty())
    return;

  StringRange newlineBeforeFunction = allText.SubString(allText.Begin(), function.Begin()).FindLastOf("\n");

  if (newlineBeforeFunction.Empty())
    return;
  
  // Increment past the newline character to be at the beginning of the line the function declaration is on
  newlineBeforeFunction.IncrementByRune();
  
  // Find the indent space leading up the start of non-whitespace text to account for
  // potential attribute tags and get the correct indent size
  StringRange textBeforeFunction = allText.SubString(newlineBeforeFunction.Begin(), function.End());
  Rune indentEndRune = textBeforeFunction.FindFirstNonWhitespaceRune();
  StringRange indentEnd = textBeforeFunction.FindFirstOf(indentEndRune);

  indent = allText.SubString(newlineBeforeFunction.Begin(), indentEnd.Begin());
  size_t bracesCount = 0;
  StringIterator endIndex;

  // The entire idea here is that we scanned up until we found the function we were calling Zero.Connect in
  // Now we're going to find the END of the function by counting { and }
  // Once we find the end, that's where we put our generated code!
  StringRange textRange(function.Begin(), allText.End());
  for (; !textRange.Empty(); textRange.PopFront())
  {
    Rune r = textRange.ReadCurrentRune();

    // if we are in a comment scan until we reach a new line or the end of the text range
    if (r == '/')
    {
      textRange.PopFront();
      if (!textRange.Empty())
      {
        r = textRange.ReadCurrentRune();
        if (r == '/')
        {
          StringRange newline = textRange.FindFirstOf("\n");
          if (!newline.Empty())
            textRange = textRange.SubString(newline.End(), textRange.End());
        }
        if (r == '*')
        {
          // Have to pop front after the star as /*/*/ is a valid c style comment
          textRange.PopFront();
          StringRange endComment = textRange.FindFirstOf("*/");
          if (!endComment.Empty())
            textRange = textRange.SubString(endComment.End(), textRange.End());
        }
      }
      // we reached the end of the text range and need to break out of the for loop too
      if (textRange.Empty())
        break;
    }

    if (r == '{')
      ++bracesCount;

    if (r == '}')
    {
      --bracesCount;

      if (bracesCount == 0)
      {
        endIndex = textRange.Begin();
        break;
      }
    }
  }

  // We have no idea where to put it!
  if (endIndex.Empty())
    return;

  // Move past the brace
  ++endIndex;
  positionOut = endIndex.mIteratorRange.Data() - allText.Data();
}

//**************************************************************************************************
void ZilchDocumentResource::ValidateNewScriptName(Status& status, StringParam name)
{
  // If we're making a new type, then we need to check if this name already exists.
  if (BoundType* existingType = MetaDatabase::GetInstance()->FindType(name))
  {
    // We can replace proxies
    if (existingType->HasAttribute(ObjectAttributes::cProxy) == false)
    {
      status.SetFailed("A type already exists by that name");
      return;
    }
  }
}

//**************************************************************************************************
void ZilchDocumentResource::ValidateRawScriptName(Status& status, StringParam name)
{
  // Make sure the user used a valid Zilch type name
  if(LibraryBuilder::CheckUpperIdentifier(name) == false)
  {
    status.SetFailed("Zilch type names must start with an uppercase letter and not contain invalid symbols");
    return;
  }
}

//**************************************************************************************************
void ZilchDocumentResource::PrepForAutoComplete(ICodeEditor* editor, Project& project, Module& dependencies)
{
  String allText = editor->GetAllText();

  project.TolerantMode = true;

  // Remove the implicit core library since we'll add it back with GetLibraries
  dependencies.Clear();

  Array<LibraryRef> libraries;
  GetLibraries(libraries);

  for (uint i = 0; i < libraries.Size(); ++i)
    dependencies.PushBack(libraries[i]);
  project.AddCodeFromString(allText, mContentItem->GetFullPath(), editor->GetDocumentResource());
}

//**************************************************************************************************
void ZilchDocumentResource::AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition)
{
  Project project;
  Module dependencies;
  PrepForAutoComplete(editor, project, dependencies);

  project.GetDefinitionInfo(dependencies, cursorPosition, mContentItem->GetFullPath(), definition);
}

//**************************************************************************************************
void ZilchDocumentResource::GetAutoCompleteInfo(ICodeEditor* editor, AutoCompleteInfo& info)
{
  Project project;
  Module dependencies;
  PrepForAutoComplete(editor, project, dependencies);
  
  String cursorOrigin = mContentItem->GetFullPath();
  project.GetAutoCompleteInfo(dependencies, editor->GetCaretPosition(), cursorOrigin, info);

  // Don't show types marked as hidden
  for (uint i = info.CompletionEntries.Size() - 1; i < info.CompletionEntries.Size(); --i)
  {
    BoundType* type = MetaDatabase::FindType(info.CompletionEntries[i].Type);
    if (type->HasAttributeInherited(ObjectAttributes::cHidden))
      info.CompletionEntries.EraseAt(i);
  }
}

//**************************************************************************************************
void ZilchDocumentResource::AddTypesToCompletion(Array<Completion>& completions, LibraryRef library)
{
  if (library != nullptr)
  {
    forRange(BoundType*& type, library->BoundTypes.Values())
    {
      // Don't show types marked as hidden
      if (type->HasAttributeInherited(ObjectAttributes::cHidden))
        continue;
      Completion& completion = completions.PushBack();
      completion.Name = type->Name;
      completion.Description = type->Description;
    }
  }
}

//**************************************************************************************************
bool ZilchDocumentResource::CanStartLocalWordCompletion(ICodeEditor* editor)
{
  using namespace Zilch;

  String keyword = GetPreviousKeyword(editor);

  if (keyword.Empty())
    return true;


  static HashSet<String> noLocalCompletionKeywords;
  static bool initialized = false;
  if (initialized == false)
  {
    initialized = true;
    noLocalCompletionKeywords.Insert(Grammar::GetKeywordOrSymbol(Grammar::Class));
    noLocalCompletionKeywords.Insert(Grammar::GetKeywordOrSymbol(Grammar::Struct));
    noLocalCompletionKeywords.Insert(Grammar::GetKeywordOrSymbol(Grammar::Variable));
    noLocalCompletionKeywords.Insert(Grammar::GetKeywordOrSymbol(Grammar::Function));
    noLocalCompletionKeywords.Insert(Grammar::GetKeywordOrSymbol(Grammar::Enumeration));
    noLocalCompletionKeywords.Insert(Grammar::GetKeywordOrSymbol(Grammar::Flags));
    noLocalCompletionKeywords.Insert(Grammar::GetKeywordOrSymbol(Grammar::Sends));
  }

  return (noLocalCompletionKeywords.Contains(keyword) == false);
}

}//namespace Zero
