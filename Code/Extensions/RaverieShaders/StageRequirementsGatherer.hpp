// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class StageRequirementsGatherer;

/// Context for stage requirements gathering. Used to store state needed during
/// iteration.
class StageRequirementsGathererContext : public Raverie::WalkerContext<StageRequirementsGatherer, StageRequirementsGathererContext>
{
public:
  HashMap<Raverie::Type*, Raverie::ClassNode*> mTypeMap;
  HashMap<Raverie::Function*, Raverie::FunctionNode*> mFunctionMap;
  HashMap<Raverie::Function*, Raverie::ConstructorNode*> mConstructorMap;
  HashMap<Raverie::Property*, Raverie::MemberVariableNode*> mVariableMap;
  HashSet<void*> mProcessedObjects;

  RaverieShaderIRLibrary* mCurrentLibrary;
  StageRequirementsData mCurrentRequirements;

  /// Raverie library to shader library map. Needed during recursion to find the
  /// shader library for a symbol to check for cached stage requirements.
  HashMap<Raverie::Library*, RaverieShaderIRLibrary*> mRaverieLibraryToShaderLibraryMap;

  ShaderCompilationErrors* mErrors;
};

/// Helper raverie AST walker to find what symbols have various stage requirements
/// and emit errors when invalid combinations are found (e.g. a vertex calling a
/// pixel only function).
class StageRequirementsGatherer
{
public:
  StageRequirementsGatherer(RaverieShaderSpirVSettings* settings);

  /// Run the requirements gatherer to find if any invalid shader stage
  /// combinations are found. Returns false if an error was found.
  bool Run(Raverie::SyntaxTree& syntaxTree, RaverieShaderIRLibrary* library, ShaderCompilationErrors* errors);

private:
  void PreWalkClassNode(Raverie::ClassNode*& node, StageRequirementsGathererContext* context);
  void PreWalkConstructor(Raverie::ConstructorNode*& node, StageRequirementsGathererContext* context);
  void PreWalkClassFunction(Raverie::FunctionNode*& node, StageRequirementsGathererContext* context);
  void PreWalkClassMemberVariable(Raverie::MemberVariableNode*& node, StageRequirementsGathererContext* context);

  void WalkClassNode(Raverie::ClassNode*& node, StageRequirementsGathererContext* context);
  void WalkClassPreconstructor(Raverie::ClassNode*& node, StageRequirementsGathererContext* context);
  void WalkClassPreconstructor(Raverie::Function* preConstructor, StageRequirementsGathererContext* context);
  void WalkClassConstructor(Raverie::ConstructorNode*& node, StageRequirementsGathererContext* context);
  void WalkClassFunction(Raverie::FunctionNode*& node, StageRequirementsGathererContext* context);
  void WalkClassMemberVariable(Raverie::MemberVariableNode*& node, StageRequirementsGathererContext* context);

  void WalkMemberAccessNode(Raverie::MemberAccessNode*& node, StageRequirementsGathererContext* context);
  void WalkStaticTypeNode(Raverie::StaticTypeNode*& node, StageRequirementsGathererContext* context);

  void MergeCurrent(RaverieShaderIRLibrary* shaderLibrary, Raverie::Member* raverieMember, Raverie::SyntaxNode* node, StageRequirementsGathererContext* context);

  void BuildLibraryMap(RaverieShaderIRLibrary* library, StageRequirementsGathererContext* context);

  ShaderStage::Enum GetRequiredStages(Raverie::Member* raverieObject, Raverie::ReflectionObject* owner);
  String GetStageName(ShaderStage::Enum stage);
  void CheckAndDispatchErrors(Raverie::Member* raverieObject, Raverie::ReflectionObject* owner, StageRequirementsGathererContext* context);

  ShaderCompilationErrors* mErrors;
  RaverieShaderSpirVSettings* mSettings;
};

} // namespace Raverie
