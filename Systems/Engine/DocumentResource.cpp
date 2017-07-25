///////////////////////////////////////////////////////////////////////////////
///
/// \file Resource.cpp
/// Implementation of the resource system class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
  DefineEvent(ClearAllAnnotations);
}

ZilchDefineType(SavingEvent, builder, type)
{
}

Completion::Completion() :
  AssociatedResource(nullptr)
{
}

Completion::Completion(StringParam name) :
  Name(name),
  AssociatedResource(nullptr)
{
}

Completion::Completion(StringParam name, StringParam description) :
  Name(name),
  Description(description),
  AssociatedResource(nullptr)
{
}

bool CallTipSorter(CallTip& a, CallTip& b)
{
  size_t aSize = a.Parameters.Size();
  size_t bSize = b.Parameters.Size();

  if (aSize < bSize)
  {
    return true;
  }
  else if (bSize < aSize)
  {
    return false;
  }

  if (a.Return < b.Return)
  {
    return true;
  }
  else if (b.Return < a.Return)
  {
    return false;
  }

  // We know the size of the parameters are equal!
  for (size_t i = 0; i < aSize; ++i)
  {
    ParameterTip& aParam = a.Parameters[i];
    ParameterTip& bParam = b.Parameters[i];

    if (aParam.Name < bParam.Name)
    {
      return true;
    }
    else if (bParam.Name < aParam.Name)
    {
      return false;
    }

    if (aParam.Type < bParam.Type)
    {
      return true;
    }
    else if (bParam.Type < aParam.Type)
    {
      return false;
    }
  }

  return false;
}

void ICodeEditor::SortCallTips(Array<CallTip>& tips)
{
  Sort(tips.All(), CallTipSorter);
}

bool SortCompletions(Completion& a, Completion& b)
{
  return a.Name < b.Name;
}

void ICodeEditor::SortAndFilterCompletions(Array<Completion>& completions)
{
  HashSet<String> uniqueNames;

  for (size_t i = 0; i < completions.Size();)
  {
    Completion& completion = completions[i];

    if (uniqueNames.Contains(completion.Name) == false)
    {
      uniqueNames.Insert(completion.Name);
    }
    else
    {
      completions.EraseAt(i);
      continue;
    }

    ++i;
  }

  Sort(completions.All(), SortCompletions);
}

StringRange ICodeInspector::GetPreviousKeyword(ICodeEditor* editor)
{
  StringRange allText = editor->GetAllText();
  StringIterator caretIt = allText.Begin() + (editor->GetCaretPosition() - 1);

  if(caretIt.Data() < allText.Data())
    return StringRange();

  // First, skip the word that we're currently on
  while(caretIt.Data() > allText.Data())
  {
    if(UTF8::IsAlphaNumeric(*caretIt))
      --caretIt;
    else
      break;
  }

  // Now skip the whitespace that comes before that word
  while(caretIt.Data() > allText.Data())
  {
    if(UTF8::IsWhiteSpace(*caretIt))
      --caretIt;
    else
      break;
  }

  StringIterator keywordEnd = caretIt;

  // Loop until we find a non [a-zA-Z0-9]
  while(caretIt.Data() > allText.Data())
  {
    if(UTF8::IsAlphaNumeric(*caretIt))
      --caretIt;
    else
      break;
  }

  // Move the beginning 1 forward, since we would have moved back too far
  // Also move the end 1 forward, since it's supposed to point 1 past the end
  return StringRange(caretIt + 1, keywordEnd + 1);
}

bool ICodeInspector::IndentOnNewlineWhenLastCharacterIs(ICodeEditor* editor, Rune added, Rune lookFor)
{
  if (added == '\n')
  {
    size_t line = editor->GetCaretLine();

    String previousLineText = editor->GetLineText(line - 1);

    if (previousLineText.FindLastNonWhitespaceRune() == lookFor)
    {
      editor->Indent(line);
      return true;
    }
  }

  return false;
}

bool ICodeInspector::CanStartLocalWordCompletion(ICodeEditor* editor)
{
  return true;
}

bool ICodeInspector::SupportsZeroConnect()
{
  return false;
}

void ICodeInspector::AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition)
{
}

void ICodeInspector::FindPositionToGenerateFunction(ICodeEditor* editor, int& positionOut, String& indent)
{
  positionOut = -1;
}

String ICodeInspector::GenerateConnectCallEnd(StringParam functionName)
{
  return String();
}

String ICodeInspector::GenerateConnectFunctionStart(StringParam functionName, StringParam eventType)
{
  return String();
}

String ICodeInspector::GenerateConnectFunctionEnd()
{
  return String();
}

bool ICodeInspector::UnindentOnCharacter(ICodeEditor* editor, Rune added, Rune lookFor)
{
  if (added == lookFor)
  {
    size_t line = editor->GetCaretLine();
    editor->Unindent(line);
    return true;
  }

  return false;
}

void ICodeInspector::AddCompletionsFromMetaType(Array<Completion>& completions, BoundType* type)
{
  // This seems slightly counter intuitive, but we actually add everything we find both from docs and from the meta type
  // We must do docs first because any duplicates will get filtered out in the completion dialog
  if(ClassDoc* classDoc = Z::gDocumentation->mClassMap.FindValue(type->Name, nullptr))
  {
    // Add all properties the ClassDoc has to the completion list
    forRange(PropertyDoc* property, classDoc->mProperties.All())
    {
      Completion& completion = completions.PushBack();
      completion.Name = property->mName;
      completion.Description = property->mDescription;
      completion.SignaturePathType = property->mType;
    }

    // add all methods the ClassDoc has to the completions list
    forRange(MethodDoc* method, classDoc->mMethods.All())
    {
      Completion& completion = completions.PushBack();
      completion.Name = method->mName;
      completion.Description = method->mDescription;

      if (method->mReturnType.Empty())
      {
        completion.SignaturePathType = BuildString(method->mName, method->mParameters);
      }
      else
      {
        completion.SignaturePathType = BuildString(method->mName, method->mParameters, " : ", method->mReturnType);
      }
    }
  }

  forRange(Property* property, type->GetProperties())
  {
    Completion& completion = completions.PushBack();
    completion.Name = property->Name;
    completion.SignaturePathType = property->PropertyType->ToString();
  }

  forRange(Function* method, type->GetFunctions())
  {
    Completion& completion = completions.PushBack();
    completion.Name = method->Name;
    completion.SignaturePathType = method->FunctionType->ToString();
  }
}

void ICodeInspector::AddCallTipParametersFromArgumentString(CallTip& tip, StringRange arguments, ArgumentOptions::Type options)
{
  String paramType;
  String paramName;
  StringIterator argStart;

  StringIterator it = arguments.Begin();
  StringIterator end = arguments.End();
  for(int i = 0; it < end; ++it, ++i)
  {
    Rune r = *it;

    if(r == ' ' || r == ',' || r == ')')
    {
      if (!argStart.Empty())
      {
        StringRange currentRange(argStart, it);

        if (options & ArgumentOptions::Typeless)
        {
          if (!paramName.Empty())
            paramType = paramName;
          
          paramName = currentRange;
        }
        else
        {
          if (paramType.Empty())
          {
            paramType = currentRange;
          }
          else
          {
            paramName = currentRange;
          }
        }

        if (r == ',' || r == ')')
        {
          ParameterTip& param = tip.Parameters.PushBack();
          param.Type = paramType;
          param.Name = paramName;
          paramType = String();
          paramName = String();
        }

        argStart = StringIterator();
      }
    }
    else if (argStart.Empty())
    {
      argStart = it;
    }
  }
}

void AddCallTipParametersFromArgumentList(CallTip &tip, const Array<ParameterDoc *> &params)
{
  for (uint i = 0; i < params.Size(); ++i)
  {
    ParameterDoc* currParam = params[i];

    ParameterTip& paramTip = tip.Parameters.PushBack();

    paramTip.Type = currParam->mType;
    paramTip.Name = currParam->mName;
    paramTip.Description = currParam->mDescription;
  }
}

void AddCallTipFromDocMethod(Array<CallTip>& tips, MethodDoc* method)
{
  CallTip& tip = tips.PushBack();

  tip.Description = method->mDescription;
  tip.Return = method->mReturnType;

  if (!method->mParameters.Empty())
  {
    AddCallTipParametersFromArgumentList(tip, method->mParameterList);
  }
}

void ICodeInspector::AddCallTipFromMetaMethod(Array<CallTip>& tips, BoundType* owner, Function* function)
{
  static const String Void("void");

  Array<MethodDoc*>* methodList = nullptr;

  if (ClassDoc* classDoc = Z::gDocumentation->mClassMap.FindValue(owner->Name, NULL))
  {
    methodList = &classDoc->mMethodsMap[function->Name];
  }


  CallTip& tip = tips.PushBack();

  MethodDoc* methodDoc = nullptr;

  if (methodList)
  {
    // find an overload of this method with correct parameters
    methodDoc = MethodDocWithSameParams(*methodList, function);

    // if we found it save the description
    if (methodDoc)
    {
      tip.Description = methodDoc->mDescription;
    }
  }

  DelegateType* delegateType = function->FunctionType;
  for (size_t i = 0; i < delegateType->Parameters.Size(); ++i)
  {
    ParameterTip& param = tip.Parameters.PushBack();
    param.Name = delegateType->Parameters[i].Name;
    param.Type = delegateType->Parameters[i].ParameterType->ToString();

    if (methodDoc && !methodDoc->mParameterList[i]->mName.Empty())
      param.Name = methodDoc->mParameterList[i]->mName;
  }
}

//------------------------------------------------------------ Document Resource
ZilchDefineType(DocumentResource, builder, type)
{
  type->AddAttribute(ObjectAttributes::cResourceInterface);

  ZeroBindDocumented();
}

DocumentResource::DocumentResource()
{
  IncludedInCrash = false;
}

DocumentResource::~DocumentResource()
{
  //Remove from the searchable map
  Z::gResources->TextResources.Erase(this->LoadPath);
}

void DocumentResource::DocumentSetup(ResourceEntry& entry, bool searchable)
{
  // If the DocumentResource is being loaded in the editor from a content item
  // use the path of the file in the content system for loading and saving.
  mContentItem = entry.mLibrarySource;
  mResourceId = entry.mResourceId;

  if(entry.mLibrarySource)
    LoadPath = entry.mLibrarySource->GetFullPath();
  else
    LoadPath = entry.FullPath;

  if(searchable)
    Z::gResources->TextResources.InsertNoOverwrite(LoadPath, mResourceId);
}

void DocumentResource::UpdateContentItem(ContentItem* contentItem)
{
  // A content item has been assigned to this resource make sure it is
  // searchable and saves to the content item file.
  mContentItem = contentItem;
  LoadPath = contentItem->GetFullPath();
  Z::gResources->TextResources.InsertNoOverwrite(LoadPath, mResourceId);
}


void DocumentResource::SetAndSaveData(StringRange data)
{
  if(mContentItem && Z::gContentSystem->mHistoryEnabled)
  {
    String backUpPath = Z::gContentSystem->GetHistoryPath(mContentItem->mLibrary);
    BackUpFile(backUpPath, LoadPath);
    WriteStringRangeToFile(LoadPath, data);
  }

  ReloadData(data);
}

namespace Events
{
  DefineEvent(Save);
  DefineEvent(SaveCheck);
}

}//namespace Zero
