// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class RaverieSpirVFrontEnd;

class RaverieSpirVFrontEndContext : public Raverie::WalkerContext<RaverieSpirVFrontEnd, RaverieSpirVFrontEndContext>
{
public:
  RaverieSpirVFrontEndContext();

  BasicBlock* GetCurrentBlock();

  void PushMergePoints(BasicBlock* continuePoint, BasicBlock* breakPoint);
  void PopMergeTargets();

  void PushIRStack(IRaverieShaderIR* ir);
  IRaverieShaderIR* PopIRStack();

  RaverieShaderIRType* mCurrentType;
  BasicBlock* mCurrentBlock;

  Array<Pair<BasicBlock*, BasicBlock*>> mMergePointStack;
  BasicBlock* mContinueTarget;
  BasicBlock* mBreakTarget;

  Array<IRaverieShaderIR*> mResultStack;
  HashMap<Raverie::Variable*, IRaverieShaderIR*> mRaverieVariableToIR;
  RaverieShaderIRFunction* mCurrentFunction;

  RaverieShaderDebugInfo mDebugInfo;

  /// Some function calls need to write some instructions after the
  /// function call (storage class differences). This data is used to keep
  /// track of the copies that need to happen afterwards.
  struct PostableCopy
  {
    PostableCopy()
    {
      mSource = mDestination = nullptr;
    }
    PostableCopy(RaverieShaderIROp* dest, RaverieShaderIROp* source)
    {
      mSource = source;
      mDestination = dest;
    }

    RaverieShaderIROp* mSource;
    RaverieShaderIROp* mDestination;
  };

  Array<PostableCopy> mFunctionPostambleCopies;
};

class RaverieSpirVFrontEnd : public BaseShaderIRTranslator
{
public:
  ~RaverieSpirVFrontEnd() override;

  // Tell the translator what settings to use for translation (Names, render
  // targets, etc...)
  void SetSettings(RaverieShaderSpirVSettingsRef& settings) override;
  void Setup() override;

  // Translate a given project (and it's syntax tree) into the passed in
  // library. Each ShaderType in the library will contain translated pieces of
  // the target language. These pieces can be put together into a final shader
  // with "BuildFinalShader".
  bool Translate(Raverie::SyntaxTree& syntaxTree, RaverieShaderIRProject* project, RaverieShaderIRLibrary* library) override;

  RaverieShaderIRType*
  MakeTypeInternal(RaverieShaderIRLibrary* shaderLibrary, ShaderIRTypeBaseType::Enum baseType, StringParam typeName, Raverie::BoundType* raverieType, spv::StorageClass storageClass);
  RaverieShaderIRType*
  MakeTypeAndPointer(RaverieShaderIRLibrary* shaderLibrary, ShaderIRTypeBaseType::Enum baseType, StringParam typeName, Raverie::BoundType* raverieType, spv::StorageClass pointerStorageClass);
  RaverieShaderIRType* MakeCoreType(
      RaverieShaderIRLibrary* shaderLibrary, ShaderIRTypeBaseType::Enum baseType, size_t components, RaverieShaderIRType* componentType, Raverie::BoundType* raverieType, bool makePointerType = true);
  RaverieShaderIRType* MakeStructType(RaverieShaderIRLibrary* shaderLibrary, StringParam typeName, Raverie::BoundType* raverieType, spv::StorageClass pointerStorageClass);
  RaverieShaderIRType*
  FindOrCreateInterfaceType(RaverieShaderIRLibrary* shaderLibrary, StringParam baseTypeName, Raverie::BoundType* raverieType, ShaderIRTypeBaseType::Enum baseType, spv::StorageClass storageClass);
  RaverieShaderIRType* FindOrCreateInterfaceType(RaverieShaderIRLibrary* shaderLibrary, Raverie::BoundType* raverieType, ShaderIRTypeBaseType::Enum baseType, spv::StorageClass storageClass);
  RaverieShaderIRType* FindOrCreatePointerInterfaceType(RaverieShaderIRLibrary* shaderLibrary, RaverieShaderIRType* valueType, spv::StorageClass storageClass);
  ShaderIRTypeMeta* MakeShaderTypeMeta(RaverieShaderIRType* shaderType, Raverie::NodeList<Raverie::AttributeNode>* nodeAttributeList);

  void ExtractRaverieAsComments(Raverie::SyntaxNode*& node, RaverieSpirVFrontEndContext* context);

  // Parse and validate attributes for a type. If no locations are available
  // then the nodes will be null (e.g. native types).
  void ParseAttributes(Raverie::Array<Raverie::Attribute>& raverieAttributes, Raverie::NodeList<Raverie::AttributeNode>* attributeNodes, ShaderIRTypeMeta* typeMeta);
  // Parse and validate attributes for a function. If no locations are available
  // then the nodes will be null (e.g. native types).
  void ParseAttributes(Raverie::Array<Raverie::Attribute>& raverieAttributes, Raverie::NodeList<Raverie::AttributeNode>* attributeNodes, ShaderIRFunctionMeta* functionMeta);
  // Parse and validate attributes for a field. If no locations are available
  // then the nodes will be null (e.g. native types).
  void ParseAttributes(Raverie::Array<Raverie::Attribute>& raverieAttributes, Raverie::NodeList<Raverie::AttributeNode>* attributeNodes, ShaderIRFieldMeta* fieldMeta);
  void ParseRaverieAttributes(Raverie::Array<Raverie::Attribute>& raverieAttributes, Raverie::NodeList<Raverie::AttributeNode>* attributeNodes, ShaderIRAttributeList& shaderAttributes);
  // Loads a raverie attribute into an IR attribute. Uses the node's location if
  // available.
  void ParseRaverieAttribute(Raverie::Attribute& raverieAttribute, Raverie::AttributeNode* attributeNode, ShaderIRAttributeList& shaderAttributes);
  // Validates that the given attribute list doesn't contain any invalid
  // attributes. Uses 'errorTypeName' to display what kind of thing (e.g.
  // field/function) owned these attributes.
  void ValidateAllowedAttributes(ShaderIRAttributeList& shaderAttributes, HashMap<String, AttributeInfo>& allowedAttributes, StringParam errorTypeName);
  void ValidateNameOverrideAttribute(ShaderIRAttribute* shaderAttribute);
  void ValidateSingleParamAttribute(ShaderIRAttribute* shaderAttribute, StringParam expectedParamName, Raverie::ConstantType::Enum expectedParamType, bool allowEmptyName);
  void ValidateAttributeNoParameters(ShaderIRAttribute* shaderAttribute);
  void ValidateAttributeParameters(ShaderIRAttribute* shaderAttribute, HashMap<String, AttributeInfo>& allowedAttributes, StringParam errorTypeName);
  bool ValidateAttributeParameterSignature(ShaderIRAttribute* shaderAttribute, const AttributeInfo::ParameterSignature& signature) const;
  bool DoTypesMatch(const AttributeInfo::ParamType& actualType, const AttributeInfo::ParamType& expectedType) const;
  // Validates that the given attribute has all dependency attributes specified
  void ValidateAttributeDependencies(ShaderIRAttribute* shaderAttribute, ShaderIRAttributeList& shaderAttributeList, Array<String>& dependencies);
  // Validates that none of the given attribute names are also present. Needed
  // to have exclusive attribute combinations.
  void ValidateAttributeExclusions(ShaderIRAttribute* shaderAttribute, ShaderIRAttributeList& shaderAttributeList, Array<String>& exclusions);
  void ValidateHardwareBuiltIn(ShaderIRFieldMeta* fieldMeta, ShaderIRAttribute* shaderAttribute, bool isInput);
  void ValidateAndParseComputeAttributeParameters(ShaderIRAttribute* shaderAttribute, ShaderIRTypeMeta* typeMeta);
  void ValidateLocalSize(ShaderIRAttributeParameter& param, int max, int& toStore);

  String BuildFunctionTypeString(Raverie::Function* raverieFunction, RaverieSpirVFrontEndContext* context);
  String BuildFunctionTypeString(Raverie::BoundType* raverieReturnType, Array<Raverie::Type*>& signature, RaverieSpirVFrontEndContext* context);
  void GenerateFunctionType(Raverie::SyntaxNode* locationNode, RaverieShaderIRFunction* function, Raverie::Function* raverieFunction, RaverieSpirVFrontEndContext* context);
  void GenerateFunctionType(
      Raverie::SyntaxNode* locationNode, RaverieShaderIRFunction* function, Raverie::BoundType* raverieReturnType, Array<Raverie::Type*>& signature, RaverieSpirVFrontEndContext* context);
  RaverieShaderIRFunction* GenerateIRFunction(Raverie::SyntaxNode* node,
                                              Raverie::NodeList<Raverie::AttributeNode>* nodeAttributeList,
                                              RaverieShaderIRType* owningType,
                                              Raverie::Function* raverieFunction,
                                              StringParam functionName,
                                              RaverieSpirVFrontEndContext* context);
  void AddImplements(Raverie::SyntaxNode* node, Raverie::Function* raverieFunction, RaverieShaderIRFunction* shaderFunction, StringParam functionName, RaverieSpirVFrontEndContext* context);

  void CollectClassTypes(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context);
  void CollectEnumTypes(Raverie::EnumNode*& node, RaverieSpirVFrontEndContext* context);

  void PreWalkClassNode(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context);
  void PreWalkTemplateTypes(RaverieSpirVFrontEndContext* context);
  void PreWalkTemplateType(Raverie::BoundType* raverieType, RaverieSpirVFrontEndContext* context);
  void PreWalkClassVariables(Raverie::MemberVariableNode*& node, RaverieSpirVFrontEndContext* context);
  void AddRuntimeArray(Raverie::MemberVariableNode* node, RaverieShaderIRType* varType, ShaderIRFieldMeta* fieldMeta, RaverieSpirVFrontEndContext* context);
  void AddGlobalVariable(Raverie::MemberVariableNode* node, RaverieShaderIRType* varType, ShaderIRFieldMeta* fieldMeta, spv::StorageClass storageClass, RaverieSpirVFrontEndContext* context);
  GlobalVariableData* CreateGlobalVariable(spv::StorageClass storageClass, RaverieShaderIRType* varType, StringParam varName, RaverieSpirVFrontEndContext* context);
  void PreWalkClassConstructor(Raverie::ConstructorNode*& node, RaverieSpirVFrontEndContext* context);
  void PreWalkClassFunction(Raverie::FunctionNode*& node, RaverieSpirVFrontEndContext* context);
  void PreWalkMainFunction(Raverie::FunctionNode*& node, RaverieSpirVFrontEndContext* context);
  void PreWalkErrorCheck(RaverieSpirVFrontEndContext* context);

  void WalkClassNode(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkClassVariables(Raverie::MemberVariableNode*& node, RaverieSpirVFrontEndContext* context);
  void GeneratePreConstructor(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context);
  void GenerateDefaultConstructor(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context);
  void GenerateDummyMemberVariable(Raverie::ClassNode*& node, RaverieSpirVFrontEndContext* context);
  void GenerateStaticVariableInitializer(Raverie::MemberVariableNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkClassConstructor(Raverie::ConstructorNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkClassFunction(Raverie::FunctionNode*& node, RaverieSpirVFrontEndContext* context);

  /// Sets selfVar to the default constructed value for the given type.
  void DefaultConstructType(Raverie::SyntaxNode* locationNode, RaverieShaderIRType* type, RaverieShaderIROp* selfVar, RaverieSpirVFrontEndContext* context);
  /// Generate the function parameters for a given function node.
  void GenerateFunctionParameters(Raverie::GenericFunctionNode* node, RaverieSpirVFrontEndContext* context);
  /// Generate the function body (statements) for a given function node. May
  /// generate an entry point if needed.
  void GenerateFunctionBody(Raverie::GenericFunctionNode* node, RaverieSpirVFrontEndContext* context);
  void GenerateEntryPoint(Raverie::GenericFunctionNode* node, RaverieShaderIRFunction* function, RaverieSpirVFrontEndContext* context);

  void WalkFunctionCallNode(Raverie::FunctionCallNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkConstructorCallNode(Raverie::FunctionCallNode*& node, Raverie::StaticTypeNode* constructorNode, RaverieSpirVFrontEndContext* context);
  void WalkMemberAccessCallNode(Raverie::FunctionCallNode*& node, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context);
  void WalkMemberAccessFunctionCallNode(Raverie::FunctionCallNode*& node, Raverie::MemberAccessNode* memberAccessNode, RaverieShaderIRFunction* shaderFunction, RaverieSpirVFrontEndContext* context);
  void WalkMemberAccessExtensionInstructionCallNode(Raverie::FunctionCallNode*& node,
                                                    Raverie::MemberAccessNode* memberAccessNode,
                                                    SpirVExtensionInstruction* extensionInstruction,
                                                    RaverieSpirVFrontEndContext* context);

  void WalkLocalVariable(Raverie::LocalVariableNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkStaticTypeOrCreationCallNode(Raverie::StaticTypeNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkExpressionInitializerNode(Raverie::ExpressionInitializerNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkUnaryOperationNode(Raverie::UnaryOperatorNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkBinaryOperationNode(Raverie::BinaryOperatorNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkCastNode(Raverie::TypeCastNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkValueNode(Raverie::ValueNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkLocalRef(Raverie::LocalVariableReferenceNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkMemberAccessNode(Raverie::MemberAccessNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkMultiExpressionNode(Raverie::MultiExpressionNode*& node, RaverieSpirVFrontEndContext* context);

  void WalkIfRootNode(Raverie::IfRootNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkIfNode(Raverie::IfNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkBreakNode(Raverie::BreakNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkContinueNode(Raverie::ContinueNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkReturnNode(Raverie::ReturnNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkWhileNode(Raverie::WhileNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkDoWhileNode(Raverie::DoWhileNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkForNode(Raverie::ForNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkForEachNode(Raverie::ForEachNode*& node, RaverieSpirVFrontEndContext* context);
  void WalkLoopNode(Raverie::LoopNode*& node, RaverieSpirVFrontEndContext* context);
  // Loop helper functions
  void WalkGenericLoop(
      Raverie::SyntaxNode* initializerNode, Raverie::SyntaxNode* iterator, Raverie::ConditionalLoopNode* conditionalNode, Raverie::LoopScopeNode* loopScopeNode, RaverieSpirVFrontEndContext* context);
  void GenerateLoopHeaderBlock(BasicBlock* headerBlock, BasicBlock* branchTarget, BasicBlock* mergeBlock, BasicBlock* continueBlock, RaverieSpirVFrontEndContext* context);
  void GenerateLoopConditionBlock(
      Raverie::ConditionalLoopNode* conditionalNode, BasicBlock* conditionBlock, BasicBlock* branchTrueBlock, BasicBlock* branchFalseBlock, RaverieSpirVFrontEndContext* context);
  void GenerateLoopStatements(Raverie::LoopScopeNode* loopScopeNode, BasicBlock* loopBlock, BasicBlock* mergeBlock, BasicBlock* continueBlock, RaverieSpirVFrontEndContext* context);
  void GenerateLoopContinueBlock(Raverie::SyntaxNode* iterator, BasicBlock* continueBlock, BasicBlock* headerBlock, RaverieSpirVFrontEndContext* context);

  /// Walk a block and make sure that it has exactly one termination condition.
  /// If there's zero then a return will be added. If there's more than one then
  /// all instructions after the first terminator will be removed.
  void FixBlockTerminators(BasicBlock* block, RaverieSpirVFrontEndContext* context);

  /// Get the setter (if available) from a member access node.
  Raverie::Function* GetSetter(Raverie::MemberAccessNode* memberAccessNode);
  /// Attempt to invert a binary op node (must be an assignment) into calling a
  /// setter.
  // If the given result node is null then the right hand side of the binary op
  // node is walked but it can be manually passed in for ops with extra
  // expressions (e.g. +=).
  bool ResolveSetter(Raverie::BinaryOperatorNode* node, RaverieShaderIROp* resultValue, Raverie::SyntaxNode* resultNode, RaverieSpirVFrontEndContext* context);

  // Helpers to perform standard binary operations
  IRaverieShaderIR* PerformBinaryOp(Raverie::BinaryOperatorNode*& node, OpType opType, RaverieSpirVFrontEndContext* context);
  IRaverieShaderIR* PerformBinaryOp(Raverie::BinaryOperatorNode*& node, OpType opType, IRaverieShaderIR* lhs, IRaverieShaderIR* rhs, RaverieSpirVFrontEndContext* context);
  void PerformBinaryAssignmentOp(Raverie::BinaryOperatorNode*& node, OpType opType, RaverieSpirVFrontEndContext* context);
  void PerformBinaryAssignmentOp(Raverie::BinaryOperatorNode*& node, OpType opType, IRaverieShaderIR* lhs, IRaverieShaderIR* rhs, RaverieSpirVFrontEndContext* context);
  // Helpers to perform standard unary operations
  IRaverieShaderIR* PerformUnaryOp(Raverie::UnaryOperatorNode*& node, OpType opType, RaverieSpirVFrontEndContext* context);
  IRaverieShaderIR* PerformUnaryIncDecOp(Raverie::UnaryOperatorNode*& node, IRaverieShaderIR* constantOne, OpType opType, RaverieSpirVFrontEndContext* context);
  // Helpers to perform standard typecast operations
  IRaverieShaderIR* PerformTypeCast(Raverie::TypeCastNode*& node, OpType opType, RaverieSpirVFrontEndContext* context);

  RaverieShaderIROp* GetIntegerConstant(int value, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* GetConstant(RaverieShaderIRType* type, StringParam value, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* GetConstant(RaverieShaderIRType* type, Raverie::Any value, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* ConstructCompositeFromScalar(BasicBlock* block, RaverieShaderIRType* compositeType, IRaverieShaderIR* scalar, RaverieSpirVFrontEndContext* context);

  /// Create a specialization constant from a member variable node.
  RaverieShaderIROp* AddSpecializationConstant(Raverie::MemberVariableNode* node, RaverieShaderIRType* varType, RaverieSpirVFrontEndContext* context);
  /// Creates a specialization constant of the given type with the given unique
  /// lookup key. If a specialization constant is not a scalar, then it must be
  /// iteratively constructed from other specialization constants. If non-null,
  /// the given literal value will be used to initialize the constant (only
  /// valid for scalars).
  RaverieShaderIROp* AddSpecializationConstantRecursively(
      void* key, RaverieShaderIRType* varType, StringParam varName, RaverieShaderIRConstantLiteral* literalValue, Raverie::CodeLocation& codeLocation, RaverieSpirVFrontEndContext* context);
  /// Create a specialization constant of a given variable type. The given key
  /// should be a unique identifier for this variable that can be used to fetch
  /// the specialization constant again at a later time. A null key specifies
  /// that there is no key (this variable can't be looked up later). This is
  /// typically used for sub-constants. The OpType should be specified to choose
  /// between scalar and composite constants.
  RaverieShaderIROp* CreateSpecializationConstant(void* key, OpType opType, RaverieShaderIRType* varType, RaverieSpirVFrontEndContext* context);

  void ToAny(RaverieShaderIRType* type, StringParam value, Raverie::Any& result);

  void ExtractDebugInfo(Raverie::SyntaxNode* node, RaverieShaderDebugInfo& debugInfo);
  void GetLineAsComment(Raverie::SyntaxNode* node, RaverieShaderIRComments& comments);
  BasicBlock* BuildBlock(StringParam labelDebugName, RaverieSpirVFrontEndContext* context);
  BasicBlock* BuildBlockNoStack(StringParam labelDebugName, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildIROpNoBlockAdd(OpType opType, RaverieShaderIRType* resultType, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildIROp(BasicBlock* block, OpType opType, RaverieShaderIRType* resultType, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildIROp(BasicBlock* block, OpType opType, RaverieShaderIRType* resultType, IRaverieShaderIR* arg0, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildIROp(BasicBlock* block, OpType opType, RaverieShaderIRType* resultType, IRaverieShaderIR* arg0, IRaverieShaderIR* arg1, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp*
  BuildIROp(BasicBlock* block, OpType opType, RaverieShaderIRType* resultType, IRaverieShaderIR* arg0, IRaverieShaderIR* arg1, IRaverieShaderIR* arg2, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildCurrentBlockIROp(OpType opType, RaverieShaderIRType* resultType, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildCurrentBlockIROp(OpType opType, RaverieShaderIRType* resultType, IRaverieShaderIR* arg0, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildCurrentBlockIROp(OpType opType, RaverieShaderIRType* resultType, IRaverieShaderIR* arg0, IRaverieShaderIR* arg1, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp*
  BuildCurrentBlockIROp(OpType opType, RaverieShaderIRType* resultType, IRaverieShaderIR* arg0, IRaverieShaderIR* arg1, IRaverieShaderIR* arg2, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildCurrentBlockAccessChain(RaverieShaderIRType* baseResultType, RaverieShaderIROp* selfInstance, IRaverieShaderIR* arg0, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp*
  BuildCurrentBlockAccessChain(RaverieShaderIRType* baseResultType, RaverieShaderIROp* selfInstance, IRaverieShaderIR* arg0, IRaverieShaderIR* arg1, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildDecorationOp(BasicBlock* block, IRaverieShaderIR* decorationTarget, spv::Decoration decorationType, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildDecorationOp(BasicBlock* block, IRaverieShaderIR* decorationTarget, spv::Decoration decorationType, int decorationValue, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildMemberDecorationOp(BasicBlock* block, IRaverieShaderIR* decorationTarget, int memberOffset, spv::Decoration decorationType, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp*
  BuildMemberDecorationOp(BasicBlock* block, IRaverieShaderIR* decorationTarget, int memberOffset, spv::Decoration decorationType, int decorationValue, RaverieSpirVFrontEndContext* context);
  RaverieShaderIRConstantLiteral* GetOrCreateConstantIntegerLiteral(int value);
  RaverieShaderIRConstantLiteral* GetOrCreateConstantLiteral(Raverie::Any value);
  RaverieShaderIROp* BuildOpVariable(RaverieShaderIRType* resultType, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* BuildOpVariable(BasicBlock* block, RaverieShaderIRType* resultType, int storageConstant, RaverieSpirVFrontEndContext* context);
  IRaverieShaderIR* WalkAndGetResult(Raverie::SyntaxNode* node, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* WalkAndGetValueTypeResult(BasicBlock* block, Raverie::SyntaxNode* node, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* WalkAndGetValueTypeResult(Raverie::SyntaxNode* node, RaverieSpirVFrontEndContext* context);
  // If this is an immediate then the op is returned. If the op is a pointer
  // than a load is generated and the load is returned.
  RaverieShaderIROp* GetOrGenerateValueTypeFromIR(BasicBlock* block, IRaverieShaderIR* instruction, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* GetOrGenerateValueTypeFromIR(IRaverieShaderIR* instruction, RaverieSpirVFrontEndContext* context);
  // If this is a pointer type (e.g. variable, parameter, etc...) then the op is
  // returned. Otherwise a new variable is generated, the immediate is stored
  // into it, and the variable is returned.
  RaverieShaderIROp* GetOrGeneratePointerTypeFromIR(BasicBlock* block, IRaverieShaderIR* instruction, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* GetOrGeneratePointerTypeFromIR(IRaverieShaderIR* instruction, RaverieSpirVFrontEndContext* context);
  // Build an op to store the source into the target. May generate OpStore or
  // OpCopyMemor depending on the type of source.
  RaverieShaderIROp* BuildStoreOp(IRaverieShaderIR* target, IRaverieShaderIR* source, RaverieSpirVFrontEndContext* context, bool forceLoadStore = true);
  RaverieShaderIROp* BuildStoreOp(BasicBlock* block, IRaverieShaderIR* target, IRaverieShaderIR* source, RaverieSpirVFrontEndContext* context, bool forceLoadStore = true);
  void GetFunctionCallArguments(Raverie::FunctionCallNode* node, Raverie::MemberAccessNode* memberAccessNode, Array<IRaverieShaderIR*>& arguments, RaverieSpirVFrontEndContext* context);
  void GetFunctionCallArguments(Raverie::FunctionCallNode* node, IRaverieShaderIR* thisOp, Array<IRaverieShaderIR*>& arguments, RaverieSpirVFrontEndContext* context);
  void WriteFunctionCallArguments(Array<IRaverieShaderIR*> arguments, RaverieShaderIRType* functionType, RaverieShaderIROp* functionCallOp, RaverieSpirVFrontEndContext* context);
  void WriteFunctionCallArgument(IRaverieShaderIR* argument, RaverieShaderIROp* functionCallOp, RaverieShaderIRType* paramType, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* GenerateFunctionCall(RaverieShaderIRFunction* shaderFunction, RaverieSpirVFrontEndContext* context);
  void WriteFunctionCallPostamble(RaverieSpirVFrontEndContext* context);
  void WriteFunctionCall(Array<IRaverieShaderIR*> arguments, RaverieShaderIRFunction* shaderFunction, RaverieSpirVFrontEndContext* context);

  // Helpers
  RaverieShaderIROp* GenerateBoolToIntegerCast(BasicBlock* block, RaverieShaderIROp* source, RaverieShaderIRType* destType, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp*
  GenerateFromBoolCast(BasicBlock* block, RaverieShaderIROp* source, RaverieShaderIRType* destType, IRaverieShaderIR* zero, IRaverieShaderIR* one, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* GenerateIntegerToBoolCast(BasicBlock* block, RaverieShaderIROp* source, RaverieShaderIRType* destType, RaverieSpirVFrontEndContext* context);
  RaverieShaderIROp* GenerateToBoolCast(BasicBlock* block, OpType op, RaverieShaderIROp* source, RaverieShaderIRType* destType, IRaverieShaderIR* zero, RaverieSpirVFrontEndContext* context);

  // Get the value result type from an an op code. If this is a pointer type it
  // will get the dereference type.
  RaverieShaderIRType* GetResultValueType(RaverieShaderIROp* op);
  // Get the pointer result type from an an op code. If this is a value type it
  // will get the pointer type.
  RaverieShaderIRType* GetPointerValueType(RaverieShaderIROp* op);
  // Helper to check if the given type contains an attribute. If the type
  // doesn't have meta this returns false.
  bool ContainsAttribute(RaverieShaderIRType* shaderType, StringParam attributeName);
  // Check if a type contains the non-copyable attribute (returns true if
  // non-copyable). Throws an error if this type is non-copyable and optionally
  // generates a dummy variable.
  bool CheckForNonCopyableType(RaverieShaderIRType* shaderType, Raverie::ExpressionNode* node, RaverieSpirVFrontEndContext* context, bool generateDummyResult = true);

  RaverieShaderIRType* FindType(Raverie::Type* type, Raverie::SyntaxNode* syntaxNode, bool reportErrors = true);
  RaverieShaderIRType* FindType(Raverie::ExpressionNode* syntaxNode, bool reportErrors = true);
  // Validate that the result type of the given instruction (must be an op) is
  // of a certain type (e.g. pointer).
  bool ValidateResultType(IRaverieShaderIR* instruction, ShaderIRTypeBaseType::Enum expectedType, Raverie::CodeLocation& codeLocation, bool throwException = true);
  /// Verifies that the given instruction can be written to. This includes
  /// checking for specialization constants.
  bool ValidateLValue(RaverieShaderIROp* op, Raverie::CodeLocation& codeLocation, bool throwException = true);

  // Generates a dummy instruction based upon the result type of the given node.
  // Used to make translation not crash after errors.
  IRaverieShaderIR* GenerateDummyIR(Raverie::ExpressionNode* node, RaverieSpirVFrontEndContext* context);
  // Send a translation error with a simple message (also marks the translation
  // as having failed)
  void SendTranslationError(Raverie::CodeLocation& codeLocation, StringParam message);
  void SendTranslationError(Raverie::CodeLocation& codeLocation, StringParam shortMsg, StringParam fullMsg);
  // Send a translation error. If the location is null then a dummy location is
  // used (e.g. native types).
  void SendTranslationError(Raverie::CodeLocation* codeLocation, StringParam message);

  typedef Raverie::BranchWalker<RaverieSpirVFrontEnd, RaverieSpirVFrontEndContext> TranslatorBranchWalker;
  TranslatorBranchWalker mWalker;
  SimpleRaverieParser mRaverieCommentParser;
  RaverieSpirVFrontEndContext* mContext;

  RaverieShaderSpirVSettingsRef mSettings;
  RaverieShaderIRLibrary* mLibrary;
  RaverieShaderIRProject* mProject;

  // Was an error ever triggered?
  bool mErrorTriggered;
};

} // namespace Raverie
