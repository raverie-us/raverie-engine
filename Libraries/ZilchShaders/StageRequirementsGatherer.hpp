// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

class StageRequirementsGatherer;

/// Context for stage requirements gathering. Used to store state needed during
/// iteration.
class StageRequirementsGathererContext
    : public Zilch::WalkerContext<StageRequirementsGatherer, StageRequirementsGathererContext>
{
public:
  HashMap<Zilch::Type*, Zilch::ClassNode*> mTypeMap;
  HashMap<Zilch::Function*, Zilch::FunctionNode*> mFunctionMap;
  HashMap<Zilch::Function*, Zilch::ConstructorNode*> mConstructorMap;
  HashMap<Zilch::Property*, Zilch::MemberVariableNode*> mVariableMap;
  HashSet<void*> mProcessedObjects;

  ZilchShaderIRLibrary* mCurrentLibrary;
  StageRequirementsData mCurrentRequirements;

  /// Zilch library to shader library map. Needed during recursion to find the
  /// shader library for a symbol to check for cached stage requirements.
  HashMap<Zilch::Library*, ZilchShaderIRLibrary*> mZilchLibraryToShaderLibraryMap;

  ShaderCompilationErrors* mErrors;
};

/// Helper zilch AST walker to find what symbols have various stage requirements
/// and emit errors when invalid combinations are found (e.g. a vertex calling a
/// pixel only function).
class StageRequirementsGatherer
{
public:
  StageRequirementsGatherer(ZilchShaderSpirVSettings* settings);

  /// Run the requirements gatherer to find if any invalid shader stage
  /// combinations are found. Returns false if an error was found.
  bool Run(Zilch::SyntaxTree& syntaxTree, ZilchShaderIRLibrary* library, ShaderCompilationErrors* errors);

private:
  void PreWalkClassNode(Zilch::ClassNode*& node, StageRequirementsGathererContext* context);
  void PreWalkConstructor(Zilch::ConstructorNode*& node, StageRequirementsGathererContext* context);
  void PreWalkClassFunction(Zilch::FunctionNode*& node, StageRequirementsGathererContext* context);
  void PreWalkClassMemberVariable(Zilch::MemberVariableNode*& node, StageRequirementsGathererContext* context);

  void WalkClassNode(Zilch::ClassNode*& node, StageRequirementsGathererContext* context);
  void WalkClassPreconstructor(Zilch::ClassNode*& node, StageRequirementsGathererContext* context);
  void WalkClassPreconstructor(Zilch::Function* preConstructor, StageRequirementsGathererContext* context);
  void WalkClassConstructor(Zilch::ConstructorNode*& node, StageRequirementsGathererContext* context);
  void WalkClassFunction(Zilch::FunctionNode*& node, StageRequirementsGathererContext* context);
  void WalkClassMemberVariable(Zilch::MemberVariableNode*& node, StageRequirementsGathererContext* context);

  void WalkMemberAccessNode(Zilch::MemberAccessNode*& node, StageRequirementsGathererContext* context);
  void WalkStaticTypeNode(Zilch::StaticTypeNode*& node, StageRequirementsGathererContext* context);

  void MergeCurrent(ZilchShaderIRLibrary* shaderLibrary,
                    Zilch::Member* zilchMember,
                    Zilch::SyntaxNode* node,
                    StageRequirementsGathererContext* context);

  void BuildLibraryMap(ZilchShaderIRLibrary* library, StageRequirementsGathererContext* context);

  ShaderStage::Enum GetRequiredStages(Zilch::Member* zilchObject, Zilch::ReflectionObject* owner);
  String GetStageName(ShaderStage::Enum stage);
  void CheckAndDispatchErrors(Zilch::Member* zilchObject,
                              Zilch::ReflectionObject* owner,
                              StageRequirementsGathererContext* context);

  ShaderCompilationErrors* mErrors;
  ZilchShaderSpirVSettings* mSettings;
};

} // namespace Zero
