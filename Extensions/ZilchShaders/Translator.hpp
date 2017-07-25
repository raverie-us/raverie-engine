///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum2(OperatorAssociativityType, LeftToRight, RightToLeft);

class ZilchShaderTranslator : public BaseShaderTranslator
{
public:
  ZilchShaderTranslator();
  ~ZilchShaderTranslator();

  void SetSettings(ZilchShaderSettingsRef& settings) override;

  void Setup() override;
  void SetupGeneric();
  void SetupWalkerRegistration();
  void SetupTokenPrecedence();

  void ParseNativeLibrary(ZilchShaderLibrary* shaderLibrary) override;
  bool Translate(Zilch::SyntaxTree& syntaxTree, ZilchShaderProject* project, ZilchShaderLibraryRef& libraryRef) override;
  // Given a shader type produce the final shader translation string
  void BuildFinalShader(ShaderType* type, ShaderTypeTranslation& shaderResult, bool generatedCodeRangeMappings = false, bool walkDependencies = true) override;

  //-------------------------------------------------------------------ZilchShaderTranslator Interface
  // Returns language version string written to the shader (probably refactor later)
  virtual String GetVersionString() = 0;
  // Sets all callbacks and other language specific translation helpers up
  virtual void SetupShaderLanguage() = 0;
  // The declaration of GeometryOutput variables are special and need to be translated
  // uniquely per language (especially since this is an intrinsic type in hlsl)
  virtual void WriteGeometryOutputVariableDeclaration(Zilch::LocalVariableNode*& node, ShaderType* variableShaderType, ZilchShaderTranslatorContext* context) = 0;
  // Writes out the main definition of a class. The main function generation is only invoked when a type has a function
  // with the attribute NameSettings::mMainAttributeTag (typically "[Main]"). The typical generation of this function
  // should construct the main type (the one that this function belongs to), copy all shader inputs into it, call the
  // type's main function, and then finally copy outputs back out for the shader stage emission.
  virtual void WriteMainForClass(Zilch::SyntaxNode* node, ShaderType* currentType, ShaderFunction* function, ZilchShaderTranslatorContext* context) = 0;

  // Helper functions to build the final shader string
  void BuildDependencies(ShaderType* currentType, HashSet<ShaderType*>& visitedTypes, Array<ShaderType*>& dependencies);
  void BuildStructsTranslation(ShaderCodeBuilder& finalBuilder, Array<ShaderType*>& dependencies, CodeRangeMapping* rootRange);
  void BuildForwardDeclarationTranslationHelper(ShaderCodeBuilder& finalBuilder, ShaderType* shaderType, ShaderFunction* shaderFunction, ScopedRangeMapping& typeForwardDeclarationRange, bool generateRanges);
  void BuildForwardDeclarationTranslation(ShaderCodeBuilder& finalBuilder, Array<ShaderType*>& dependencies, ShaderType* mainType, CodeRangeMapping* rootRange);
  void BuildGlobalVarsTranslationHelper(ShaderCodeBuilder& finalBuilder, ShaderField* field, ScopedRangeMapping& typeGlobalVarsRange, bool generateRanges);
  void BuildGlobalVarsTranslation(ShaderCodeBuilder& finalBuilder, Array<ShaderType*>& dependencies, CodeRangeMapping* rootRange);
  void BuildFunctionCodeTranslationHelper(ShaderCodeBuilder& finalBuilder, ShaderType* shaderType, ShaderFunction* shaderFunction, ScopedRangeMapping& typeCodeRange, bool generateRanges);
  void BuildFunctionCodeTranslation(ShaderCodeBuilder& finalBuilder, Array<ShaderType*>& dependencies, ShaderType* mainType, CodeRangeMapping* rootRange);

  void FormatCommentsAndLines(Zilch::SyntaxNode*& node, ZilchShaderTranslatorContext* context);
  void FormatStatement(Zilch::StatementNode*& node, ZilchShaderTranslatorContext* context);

  // Given an attribute parameter, validate that it is the expected type and throw an error if it is not
  bool ValidateParamType(Zilch::AttributeParameter& attributeParam, Zilch::ConstantType::Enum expectedType, Zilch::CodeLocation& location);
  bool ValidateParamType(Zilch::ValueNode* valueNode, StringParam paramName, Zilch::Grammar::Enum expectedType, StringParam expectedTypeStr, Zilch::CodeLocation& location);
  // Parse the Intrinsic attribute from a class node
  void ParseIntrinsicAttribute(Zilch::ClassNode*& node, ShaderType* currentType);
  void ParseSharedAttribute(Zilch::MemberVariableNode*& node, ShaderAttribute* attribute, ShaderField* currentField);
  // Pre-walk functions are to gather information we need without worrying about order issues
  void PreWalkClassDeclaration(Zilch::ClassNode*& node, ZilchShaderTranslatorContext* context);
  void PreWalkClassFunction(Zilch::FunctionNode*& node, ZilchShaderTranslatorContext* context);
  void PreWalkClassVariables(Zilch::MemberVariableNode*& node, ZilchShaderTranslatorContext* context);

  void GenerateClassDeclaration(Zilch::ClassNode*& node, ZilchShaderTranslatorContext* context);
  void GenerateDummyMemberVariable(ShaderType* shaderType);
  void GeneratePreConstructor(Zilch::ClassNode*& node, ZilchShaderTranslatorContext* context);
  void GenerateDefaultConstructor(Zilch::ClassNode*& node, ZilchShaderTranslatorContext* context);
  void WalkClassVariables(Zilch::MemberVariableNode*& node, ZilchShaderTranslatorContext* context);
  void WalkClassConstructor(Zilch::ConstructorNode*& node, ZilchShaderTranslatorContext* context);
  void WalkClassFunction(Zilch::FunctionNode*& node, ZilchShaderTranslatorContext* context);

  void WalkFunctionCallNode(Zilch::FunctionCallNode*& node, ZilchShaderTranslatorContext* context);

  void WalkLocalVariable(Zilch::LocalVariableNode*& node, ZilchShaderTranslatorContext* context);
  void WalkStaticTypeOrCreationCallNode(Zilch::StaticTypeNode*& node, ZilchShaderTranslatorContext* context);
  void WalkExpressionInitializerNode(Zilch::ExpressionInitializerNode*& node, ZilchShaderTranslatorContext* context);
  void WalkUnaryOperationNode(Zilch::UnaryOperatorNode*& node, ZilchShaderTranslatorContext* context);
  void WalkBinaryOperationNode(Zilch::BinaryOperatorNode*& node, ZilchShaderTranslatorContext* context);
  void WalkCastNode(Zilch::TypeCastNode*& node, ZilchShaderTranslatorContext* context);
  void WalkValueNode(Zilch::ValueNode*& node, ZilchShaderTranslatorContext* context);
  void WalkLocalRef(Zilch::LocalVariableReferenceNode*& node, ZilchShaderTranslatorContext* context);
  void WalkGetterSetter(Zilch::MemberAccessNode*& node, Zilch::GetterSetter* getSet, ZilchShaderTranslatorContext* context);
  void WalkMemberAccessNode(Zilch::MemberAccessNode*& node, ZilchShaderTranslatorContext* context);
  void WalkMultiExpressionNode(Zilch::MultiExpressionNode*& node, ZilchShaderTranslatorContext* context);

  void WalkIfRootNode(Zilch::IfRootNode*& node, ZilchShaderTranslatorContext* context);
  void WalkIfNode(Zilch::IfNode*& node, ZilchShaderTranslatorContext* context);
  void WalkBreakNode(Zilch::BreakNode*& node, ZilchShaderTranslatorContext* context);
  void WalkContinueNode(Zilch::ContinueNode*& node, ZilchShaderTranslatorContext* context);
  void WalkReturnNode(Zilch::ReturnNode*& node, ZilchShaderTranslatorContext* context);
  void WalkWhileNode(Zilch::WhileNode*& node, ZilchShaderTranslatorContext* context);
  void WalkDoWhileNode(Zilch::DoWhileNode*& node, ZilchShaderTranslatorContext* context);
  void WalkForNode(Zilch::ForNode*& node, ZilchShaderTranslatorContext* context);
  void WalkForEachNode(Zilch::ForEachNode*& node, ZilchShaderTranslatorContext* context);
  void WalkUnknownNode(Zilch::SyntaxNode*& node, ZilchShaderTranslatorContext* context);

  void WriteStatements(Zilch::NodeList<Zilch::StatementNode>& statements, ZilchShaderTranslatorContext* context);
  void WriteFunctionStatements(Zilch::GenericFunctionNode* node, ZilchShaderTranslatorContext* context);
  void WriteFunctionSignature(Zilch::GenericFunctionNode* node, ZilchShaderTranslatorContext* context);
  void WriteFunctionCall(StringParam functionName, Zilch::NodeList<Zilch::ExpressionNode>& arguments, String* firstParam, String* lastParam, ZilchShaderTranslatorContext* context);
  void WriteFunctionCall(Zilch::NodeList<Zilch::ExpressionNode>& arguments, String* firstParam, String* lastParam, ZilchShaderTranslatorContext* context);
  void AddSelfParameter(Zilch::Type* zilchSelfType, ZilchShaderTranslatorContext* context);
  void AddSelfParameter(Zilch::Type* zilchSelfType, StringParam selfType, ZilchShaderTranslatorContext* context);
  String ApplyValueReplacement(StringParam value);
  String ApplyVariableReplacement(StringParam varName);
  String MangleName(StringParam name, Zilch::Type* classType);
  String MangleName(StringParam name, ShaderType* type);
  String FixClassName(Zilch::Type* type);
  String GenerateDefaultConstructorString(Zilch::Type* type, ZilchShaderTranslatorContext* context);
  bool IsTemplateType(Zilch::BoundType* type);
  void RegisterLibraryBoundTemplateTypes();
  void CheckForAllowedFragmentIntrinsicTypes(Zilch::ExpressionNode* node, ZilchShaderTranslatorContext* context);
  bool ContainsAttribute(Array<Zilch::Attribute>& attributes, StringParam attributeToSearch);
  bool IsInOwningLibrary(Zilch::Library* library);
  bool IsInOwningLibrary(Zilch::Type* type);
  ShaderType* FindShaderType(Zilch::Type* type, Zilch::SyntaxNode* syntaxNode, bool reportErrors = true);
  ShaderType* FindAndReportUnknownType(Zilch::Type* type, Zilch::SyntaxNode* syntaxNode);
  // Is this field considered a built-in? This is only currently needed to deal with built-in forced
  // static types like samplers as they can't follow the regular data input flow.
  bool IsBuiltInField(ShaderField* field);

  // Send a translation error with a simple message (also marks the translation as having failed)
  void SendTranslationError(Zilch::CodeLocation& codeLocation, StringParam message);
  void SendTranslationError(Zilch::CodeLocation& codeLocation, StringParam shortMsg, StringParam fullMsg);

  // Information operators need to help determine if parenthesis are needed when outputting expressions.
  // The associativity of the operator is also needed when two operators have the same precedence.
  struct OperatorInfo
  {
    OperatorInfo()
    {
      mPrecedence = 0;
      mAssociativityType = OperatorAssociativityType::LeftToRight;
    }
    OperatorInfo(int precedence, OperatorAssociativityType::Enum associtivityType)
    {
      mPrecedence = precedence;
      mAssociativityType = associtivityType;
    }

    int mPrecedence;
    OperatorAssociativityType::Enum mAssociativityType;
  };
  HashMap<int, OperatorInfo> mTokenPrecedence;

  // Settings including names, render target definitions, etc...
  ZilchShaderSettingsRef mSettings;

  // Store all the walkers
  typedef Zilch::BranchWalker<ZilchShaderTranslator, ZilchShaderTranslatorContext> TranslatorBranchWalker;
  TranslatorBranchWalker mPreWalker;
  TranslatorBranchWalker mWalker;

  // The class we use to report errors. This is typically the shader project that is being compiled.
  ShaderCompilationErrors* mErrors;
  LibraryTranslator mLibraryTranslator;

  ZilchShaderProject* mCurrentProject;
  ZilchShaderLibrary* mCurrentLibrary;
  
  // Quick fix variable to allow the same translator to be used more than once. 
  // This is used to prevent registering branch walkers multiple times.
  bool mInitialized;
  // Was an error ever triggered?
  bool mErrorTriggered;
};

}//namespace Zero
