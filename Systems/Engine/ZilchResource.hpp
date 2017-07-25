////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------------------------- Zilch Library Resource
class ZilchLibraryResource : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  virtual void AddToProject(Project& project) = 0;
};

//----------------------------------------------------------------------------------- Zilch Document
class ZilchDocumentResource : public DocumentResource, public ICodeInspector
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ZilchDocumentResource();

  // DocumentResource Interface.
  StringRange LoadTextData() override;
  void ReloadData(StringRange data) override;
  String GetFormat() override;
  ICodeInspector* GetCodeInspector() override { return this; }

  // ICodeInspector Interface.
  void OnCharacterAdded(ICodeEditor* editor, Rune rune) override;
  void OnShowAutoComplete(ICodeEditor* editor) override;
  String GetSingleLineCommentToken() override;
  void GetKeywords(Array<Completion>& keywordsOut) override;
  bool CanStartLocalWordCompletion(ICodeEditor* editor) override;
  bool SupportsZeroConnect() override;
  String GenerateConnectCallEnd(StringParam functionName) override;
  String GenerateConnectFunctionStart(StringParam functionName, StringParam eventType) override;
  String GenerateConnectFunctionEnd() override;
  void FindPositionToGenerateFunction(ICodeEditor* editor, int& positionOut, String& indent) override;
  void AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition) override;

  static void ValidateScriptName(Status& status, StringParam name);

  // Fill a project with a single document that the code editor is looking at
  // and push all the proper required libraries into the module
  void PrepForAutoComplete(ICodeEditor* editor, Project& project, Module& dependencies);
  void GetAutoCompleteInfo(ICodeEditor* editor, AutoCompleteInfo& info);
  void AddTypesToCompletion(Array<Completion>& completions, LibraryRef library);

  // Get whatever libraries should be included in the code inspection
  virtual void GetLibraries(Array<LibraryRef>& libraries) = 0;

  // Text for script.
  String mText;
};

}//namespace Zero
