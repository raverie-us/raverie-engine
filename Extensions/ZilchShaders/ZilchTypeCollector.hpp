///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ZilchTypeCollectorContext
class ZilchTypeCollectorContext : public Zilch::WalkerContext<ZilchTypeCollector, ZilchTypeCollectorContext>
{
public:
  ZilchTypeCollectorContext();

  ShaderType* mCurrentType;
  ZilchShaderLibrary* mCurrentLibrary;
  Zilch::Module* mZilchModule;
  ShaderCompilationErrors* mErrors;
};

//-------------------------------------------------------------------ZilchTypeCollector
// Populates the given library with all of the necessary type information for translation/compositing.
class ZilchTypeCollector
{
public:

  // Collects all of the types from the given syntax tree and adds them to given library
  void CollectTypes(Zilch::SyntaxTree& syntaxTree, ZilchShaderLibraryRef& libraryRef, Zilch::Module* module, ShaderCompilationErrors* errors, ZilchShaderSettingsRef& settings);

private:
  void WalkClassDeclaration(Zilch::ClassNode*& node, ZilchTypeCollectorContext* context);
  void WalkClassPreConstructor(ZilchTypeCollectorContext* context, Zilch::Function* preConstructorFn);
  void WalkClassConstructor(Zilch::ConstructorNode*& node, ZilchTypeCollectorContext* context);
  void WalkClassFunction(Zilch::FunctionNode*& node, ZilchTypeCollectorContext* context);
  void WalkClassMemberVariables(Zilch::MemberVariableNode*& node, ZilchTypeCollectorContext* context);

  typedef Zilch::NodeList<Zilch::AttributeNode> AttributeNodeList;
  typedef Zilch::Array<Zilch::Attribute> AttributeArray;

  void AddFunction(Zilch::GenericFunctionNode* node, StringParam functionName, StringParam returnType, AttributeNodeList& attributeNodes, AttributeArray& attributes, ZilchTypeCollectorContext* context);
  void GenerateDefaultConstructor(Zilch::ClassNode* node, ZilchTypeCollectorContext* context);
  void AddImplements(Zilch::GenericFunctionNode* node, ShaderFunction* shaderFunction, Zilch::BoundType* boundType, ZilchTypeCollectorContext* context);
  void AddExtension(Zilch::GenericFunctionNode* node, ShaderFunction* shaderFunction, StringParam extensionType, ZilchTypeCollectorContext* context);

  //-------------------------------------------------------------------Attribute parsing and validation
  void ValidateAttribute(ShaderType* shaderType, Zilch::AttributeNode* attributeNode, ShaderAttribute& shaderAttribute, ZilchTypeCollectorContext* context);
  void ValidateAttribute(ShaderField* shaderField, Zilch::AttributeNode* attributeNode, ShaderAttribute& shaderAttribute, ZilchTypeCollectorContext* context);
  void ValidateAttribute(ShaderFunction* shaderFunction, Zilch::AttributeNode* attributeNode, ShaderAttribute& shaderAttribute, ZilchTypeCollectorContext* context);
  void ValidateBuiltIns(ShaderField* shaderField, ZilchTypeCollectorContext* context);
  void ValidateAllAttributes(ShaderType* shaderType, ZilchTypeCollectorContext* context);
  void ValidateAllAttributes(ShaderField* shaderField, ZilchTypeCollectorContext* context);
  void ValidateAllAttributes(ShaderFunction* shaderFunction, ZilchTypeCollectorContext* context);
  bool ValidateParamType(Zilch::AttributeParameter& attributeParam, Zilch::ConstantType::Enum expectedType, Zilch::CodeLocation& location, ShaderCompilationErrors* errors);
  void CountAttributes(HashMap<String, int>& attributeCounts, ShaderAttributeList& attributes);
  void ValidateAttributeCount(HashMap<String, int>& attributeCounts, StringParam attributeName, int expectedCount, StringParam typeName, ZilchTypeCollectorContext* context);
  void ValidateAttributeMaxCount(HashMap<String, int>& attributeCounts, StringParam attributeName, int maxAllowed, StringParam typeName, ZilchTypeCollectorContext* context);

  template <typename Type>
  void ParseAttributes(AttributeNodeList& attributeNodes, AttributeArray& zilchAttributes, Type* type, ZilchTypeCollectorContext* context);

  ZilchShaderSettingsRef mSettings;
};

}//namespace Zero
