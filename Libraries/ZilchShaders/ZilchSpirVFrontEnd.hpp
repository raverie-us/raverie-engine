// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class ZilchSpirVFrontEnd;

class ZilchSpirVFrontEndContext : public Zilch::WalkerContext<ZilchSpirVFrontEnd, ZilchSpirVFrontEndContext>
{
public:
  ZilchSpirVFrontEndContext();

  BasicBlock* GetCurrentBlock();

  void PushMergePoints(BasicBlock* continuePoint, BasicBlock* breakPoint);
  void PopMergeTargets();

  void PushIRStack(IZilchShaderIR* ir);
  IZilchShaderIR* PopIRStack();

  ZilchShaderIRType* mCurrentType;
  BasicBlock* mCurrentBlock;

  Array<Pair<BasicBlock*, BasicBlock*>> mMergePointStack;
  BasicBlock* mContinueTarget;
  BasicBlock* mBreakTarget;

  Array<IZilchShaderIR*> mResultStack;
  HashMap<Zilch::Variable*, IZilchShaderIR*> mZilchVariableToIR;
  ZilchShaderIRFunction* mCurrentFunction;

  ZilchShaderDebugInfo mDebugInfo;

  /// Some function calls need to write some instructions after the
  /// function call (storage class differences). This data is used to keep
  /// track of the copies that need to happen afterwards.
  struct PostableCopy
  {
    PostableCopy()
    {
      mSource = mDestination = nullptr;
    }
    PostableCopy(ZilchShaderIROp* dest, ZilchShaderIROp* source)
    {
      mSource = source;
      mDestination = dest;
    }

    ZilchShaderIROp* mSource;
    ZilchShaderIROp* mDestination;
  };

  Array<PostableCopy> mFunctionPostambleCopies;
};

class ZilchSpirVFrontEnd : public BaseShaderIRTranslator
{
public:
  ~ZilchSpirVFrontEnd() override;

  // Tell the translator what settings to use for translation (Names, render
  // targets, etc...)
  void SetSettings(ZilchShaderSpirVSettingsRef& settings) override;
  void Setup() override;

  // Translate a given project (and it's syntax tree) into the passed in
  // library. Each ShaderType in the library will contain translated pieces of
  // the target language. These pieces can be put together into a final shader
  // with "BuildFinalShader".
  bool Translate(Zilch::SyntaxTree& syntaxTree, ZilchShaderIRProject* project, ZilchShaderIRLibrary* library) override;

  ZilchShaderIRType* MakeTypeInternal(ZilchShaderIRLibrary* shaderLibrary,
                                      ShaderIRTypeBaseType::Enum baseType,
                                      StringParam typeName,
                                      Zilch::BoundType* zilchType,
                                      spv::StorageClass storageClass);
  ZilchShaderIRType* MakeTypeAndPointer(ZilchShaderIRLibrary* shaderLibrary,
                                        ShaderIRTypeBaseType::Enum baseType,
                                        StringParam typeName,
                                        Zilch::BoundType* zilchType,
                                        spv::StorageClass pointerStorageClass);
  ZilchShaderIRType* MakeCoreType(ZilchShaderIRLibrary* shaderLibrary,
                                  ShaderIRTypeBaseType::Enum baseType,
                                  size_t components,
                                  ZilchShaderIRType* componentType,
                                  Zilch::BoundType* zilchType,
                                  bool makePointerType = true);
  ZilchShaderIRType* MakeStructType(ZilchShaderIRLibrary* shaderLibrary,
                                    StringParam typeName,
                                    Zilch::BoundType* zilchType,
                                    spv::StorageClass pointerStorageClass);
  ZilchShaderIRType* FindOrCreateInterfaceType(ZilchShaderIRLibrary* shaderLibrary,
                                               StringParam baseTypeName,
                                               Zilch::BoundType* zilchType,
                                               ShaderIRTypeBaseType::Enum baseType,
                                               spv::StorageClass storageClass);
  ZilchShaderIRType* FindOrCreateInterfaceType(ZilchShaderIRLibrary* shaderLibrary,
                                               Zilch::BoundType* zilchType,
                                               ShaderIRTypeBaseType::Enum baseType,
                                               spv::StorageClass storageClass);
  ZilchShaderIRType* FindOrCreatePointerInterfaceType(ZilchShaderIRLibrary* shaderLibrary,
                                                      ZilchShaderIRType* valueType,
                                                      spv::StorageClass storageClass);
  ShaderIRTypeMeta* MakeShaderTypeMeta(ZilchShaderIRType* shaderType,
                                       Zilch::NodeList<Zilch::AttributeNode>* nodeAttributeList);

  void ExtractZilchAsComments(Zilch::SyntaxNode*& node, ZilchSpirVFrontEndContext* context);

  // Parse and validate attributes for a type. If no locations are available
  // then the nodes will be null (e.g. native types).
  void ParseAttributes(Zilch::Array<Zilch::Attribute>& zilchAttributes,
                       Zilch::NodeList<Zilch::AttributeNode>* attributeNodes,
                       ShaderIRTypeMeta* typeMeta);
  // Parse and validate attributes for a function. If no locations are available
  // then the nodes will be null (e.g. native types).
  void ParseAttributes(Zilch::Array<Zilch::Attribute>& zilchAttributes,
                       Zilch::NodeList<Zilch::AttributeNode>* attributeNodes,
                       ShaderIRFunctionMeta* functionMeta);
  // Parse and validate attributes for a field. If no locations are available
  // then the nodes will be null (e.g. native types).
  void ParseAttributes(Zilch::Array<Zilch::Attribute>& zilchAttributes,
                       Zilch::NodeList<Zilch::AttributeNode>* attributeNodes,
                       ShaderIRFieldMeta* fieldMeta);
  void ParseZilchAttributes(Zilch::Array<Zilch::Attribute>& zilchAttributes,
                            Zilch::NodeList<Zilch::AttributeNode>* attributeNodes,
                            ShaderIRAttributeList& shaderAttributes);
  // Loads a zilch attribute into an IR attribute. Uses the node's location if
  // available.
  void ParseZilchAttribute(Zilch::Attribute& zilchAttribute,
                           Zilch::AttributeNode* attributeNode,
                           ShaderIRAttributeList& shaderAttributes);
  // Validates that the given attribute list doesn't contain any invalid
  // attributes. Uses 'errorTypeName' to display what kind of thing (e.g.
  // field/function) owned these attributes.
  void ValidateAllowedAttributes(ShaderIRAttributeList& shaderAttributes,
                                 HashMap<String, AttributeInfo>& allowedAttributes,
                                 StringParam errorTypeName);
  void ValidateNameOverrideAttribute(ShaderIRAttribute* shaderAttribute);
  void ValidateSingleParamAttribute(ShaderIRAttribute* shaderAttribute,
                                    StringParam expectedParamName,
                                    Zilch::ConstantType::Enum expectedParamType,
                                    bool allowEmptyName);
  void ValidateAttributeNoParameters(ShaderIRAttribute* shaderAttribute);
  // Validates that the given attribute has all dependency attributes specified
  void ValidateAttributeDependencies(ShaderIRAttribute* shaderAttribute,
                                     ShaderIRAttributeList& shaderAttributeList,
                                     Array<String>& dependencies);
  // Validates that none of the given attribute names are also present. Needed
  // to have exclusive attribute combinations.
  void ValidateAttributeExclusions(ShaderIRAttribute* shaderAttribute,
                                   ShaderIRAttributeList& shaderAttributeList,
                                   Array<String>& exclusions);
  void ValidateHardwareBuiltIn(ShaderIRFieldMeta* fieldMeta, ShaderIRAttribute* shaderAttribute, bool isInput);
  void ValidateAndParseComputeAttributeParameters(ShaderIRAttribute* shaderAttribute, ShaderIRTypeMeta* typeMeta);
  void ValidateLocalSize(ShaderIRAttributeParameter& param, int max, int& toStore);

  String BuildFunctionTypeString(Zilch::Function* zilchFunction, ZilchSpirVFrontEndContext* context);
  String BuildFunctionTypeString(Zilch::BoundType* zilchReturnType,
                                 Array<Zilch::Type*>& signature,
                                 ZilchSpirVFrontEndContext* context);
  void GenerateFunctionType(Zilch::SyntaxNode* locationNode,
                            ZilchShaderIRFunction* function,
                            Zilch::Function* zilchFunction,
                            ZilchSpirVFrontEndContext* context);
  void GenerateFunctionType(Zilch::SyntaxNode* locationNode,
                            ZilchShaderIRFunction* function,
                            Zilch::BoundType* zilchReturnType,
                            Array<Zilch::Type*>& signature,
                            ZilchSpirVFrontEndContext* context);
  ZilchShaderIRFunction* GenerateIRFunction(Zilch::SyntaxNode* node,
                                            Zilch::NodeList<Zilch::AttributeNode>* nodeAttributeList,
                                            ZilchShaderIRType* owningType,
                                            Zilch::Function* zilchFunction,
                                            StringParam functionName,
                                            ZilchSpirVFrontEndContext* context);
  void AddImplements(Zilch::SyntaxNode* node,
                     Zilch::Function* zilchFunction,
                     ZilchShaderIRFunction* shaderFunction,
                     StringParam functionName,
                     ZilchSpirVFrontEndContext* context);

  void CollectClassTypes(Zilch::ClassNode*& node, ZilchSpirVFrontEndContext* context);
  void CollectEnumTypes(Zilch::EnumNode*& node, ZilchSpirVFrontEndContext* context);

  void PreWalkClassNode(Zilch::ClassNode*& node, ZilchSpirVFrontEndContext* context);
  void PreWalkTemplateTypes(ZilchSpirVFrontEndContext* context);
  void PreWalkTemplateType(Zilch::BoundType* zilchType, ZilchSpirVFrontEndContext* context);
  void PreWalkClassVariables(Zilch::MemberVariableNode*& node, ZilchSpirVFrontEndContext* context);
  void AddRuntimeArray(Zilch::MemberVariableNode* node, ZilchShaderIRType* varType, ZilchSpirVFrontEndContext* context);
  void AddGlobalVariable(Zilch::MemberVariableNode* node,
                         ZilchShaderIRType* varType,
                         spv::StorageClass storageClass,
                         ZilchSpirVFrontEndContext* context);
  void PreWalkClassConstructor(Zilch::ConstructorNode*& node, ZilchSpirVFrontEndContext* context);
  void PreWalkClassFunction(Zilch::FunctionNode*& node, ZilchSpirVFrontEndContext* context);
  void PreWalkMainFunction(Zilch::FunctionNode*& node, ZilchSpirVFrontEndContext* context);
  void PreWalkErrorCheck(ZilchSpirVFrontEndContext* context);

  void WalkClassNode(Zilch::ClassNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkClassVariables(Zilch::MemberVariableNode*& node, ZilchSpirVFrontEndContext* context);
  void GeneratePreConstructor(Zilch::ClassNode*& node, ZilchSpirVFrontEndContext* context);
  void GenerateDefaultConstructor(Zilch::ClassNode*& node, ZilchSpirVFrontEndContext* context);
  void GenerateDummyMemberVariable(Zilch::ClassNode*& node, ZilchSpirVFrontEndContext* context);
  void GenerateStaticVariableInitializer(Zilch::MemberVariableNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkClassConstructor(Zilch::ConstructorNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkClassFunction(Zilch::FunctionNode*& node, ZilchSpirVFrontEndContext* context);

  /// Sets selfVar to the default constructed value for the given type.
  void DefaultConstructType(Zilch::SyntaxNode* locationNode,
                            ZilchShaderIRType* type,
                            ZilchShaderIROp* selfVar,
                            ZilchSpirVFrontEndContext* context);
  /// Generate the function parameters for a given function node.
  void GenerateFunctionParameters(Zilch::GenericFunctionNode* node, ZilchSpirVFrontEndContext* context);
  /// Generate the function body (statements) for a given function node. May
  /// generate an entry point if needed.
  void GenerateFunctionBody(Zilch::GenericFunctionNode* node, ZilchSpirVFrontEndContext* context);
  void GenerateEntryPoint(Zilch::GenericFunctionNode* node,
                          ZilchShaderIRFunction* function,
                          ZilchSpirVFrontEndContext* context);

  void WalkFunctionCallNode(Zilch::FunctionCallNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkConstructorCallNode(Zilch::FunctionCallNode*& node,
                               Zilch::StaticTypeNode* constructorNode,
                               ZilchSpirVFrontEndContext* context);
  void WalkMemberAccessCallNode(Zilch::FunctionCallNode*& node,
                                Zilch::MemberAccessNode* memberAccessNode,
                                ZilchSpirVFrontEndContext* context);
  void WalkMemberAccessFunctionCallNode(Zilch::FunctionCallNode*& node,
                                        Zilch::MemberAccessNode* memberAccessNode,
                                        ZilchShaderIRFunction* shaderFunction,
                                        ZilchSpirVFrontEndContext* context);
  void WalkMemberAccessExtensionInstructionCallNode(Zilch::FunctionCallNode*& node,
                                                    Zilch::MemberAccessNode* memberAccessNode,
                                                    SpirVExtensionInstruction* extensionInstruction,
                                                    ZilchSpirVFrontEndContext* context);

  void WalkLocalVariable(Zilch::LocalVariableNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkStaticTypeOrCreationCallNode(Zilch::StaticTypeNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkExpressionInitializerNode(Zilch::ExpressionInitializerNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkUnaryOperationNode(Zilch::UnaryOperatorNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkBinaryOperationNode(Zilch::BinaryOperatorNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkCastNode(Zilch::TypeCastNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkValueNode(Zilch::ValueNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkLocalRef(Zilch::LocalVariableReferenceNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkMemberAccessNode(Zilch::MemberAccessNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkMultiExpressionNode(Zilch::MultiExpressionNode*& node, ZilchSpirVFrontEndContext* context);

  void WalkIfRootNode(Zilch::IfRootNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkIfNode(Zilch::IfNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkBreakNode(Zilch::BreakNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkContinueNode(Zilch::ContinueNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkReturnNode(Zilch::ReturnNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkWhileNode(Zilch::WhileNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkDoWhileNode(Zilch::DoWhileNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkForNode(Zilch::ForNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkForEachNode(Zilch::ForEachNode*& node, ZilchSpirVFrontEndContext* context);
  void WalkLoopNode(Zilch::LoopNode*& node, ZilchSpirVFrontEndContext* context);
  // Loop helper functions
  void WalkGenericLoop(Zilch::SyntaxNode* initializerNode,
                       Zilch::SyntaxNode* iterator,
                       Zilch::ConditionalLoopNode* conditionalNode,
                       Zilch::LoopScopeNode* loopScopeNode,
                       ZilchSpirVFrontEndContext* context);
  void GenerateLoopHeaderBlock(BasicBlock* headerBlock,
                               BasicBlock* branchTarget,
                               BasicBlock* mergeBlock,
                               BasicBlock* continueBlock,
                               ZilchSpirVFrontEndContext* context);
  void GenerateLoopConditionBlock(Zilch::ConditionalLoopNode* conditionalNode,
                                  BasicBlock* conditionBlock,
                                  BasicBlock* branchTrueBlock,
                                  BasicBlock* branchFalseBlock,
                                  ZilchSpirVFrontEndContext* context);
  void GenerateLoopStatements(Zilch::LoopScopeNode* loopScopeNode,
                              BasicBlock* loopBlock,
                              BasicBlock* mergeBlock,
                              BasicBlock* continueBlock,
                              ZilchSpirVFrontEndContext* context);
  void GenerateLoopContinueBlock(Zilch::SyntaxNode* iterator,
                                 BasicBlock* continueBlock,
                                 BasicBlock* headerBlock,
                                 ZilchSpirVFrontEndContext* context);

  /// Walk a block and make sure that it has exactly one termination condition.
  /// If there's zero then a return will be added. If there's more than one then
  /// all instructions after the first terminator will be removed.
  void FixBlockTerminators(BasicBlock* block, ZilchSpirVFrontEndContext* context);

  /// Get the setter (if available) from a member access node.
  Zilch::Function* GetSetter(Zilch::MemberAccessNode* memberAccessNode);
  /// Attempt to invert a binary op node (must be an assignment) into calling a
  /// setter.
  // If the given result node is null then the right hand side of the binary op
  // node is walked but it can be manually passed in for ops with extra
  // expressions (e.g. +=).
  bool ResolveSetter(Zilch::BinaryOperatorNode* node,
                     ZilchShaderIROp* resultValue,
                     Zilch::SyntaxNode* resultNode,
                     ZilchSpirVFrontEndContext* context);

  // Helpers to perform standard binary operations
  IZilchShaderIR* PerformBinaryOp(Zilch::BinaryOperatorNode*& node, OpType opType, ZilchSpirVFrontEndContext* context);
  IZilchShaderIR* PerformBinaryOp(Zilch::BinaryOperatorNode*& node,
                                  OpType opType,
                                  IZilchShaderIR* lhs,
                                  IZilchShaderIR* rhs,
                                  ZilchSpirVFrontEndContext* context);
  void PerformBinaryAssignmentOp(Zilch::BinaryOperatorNode*& node, OpType opType, ZilchSpirVFrontEndContext* context);
  void PerformBinaryAssignmentOp(Zilch::BinaryOperatorNode*& node,
                                 OpType opType,
                                 IZilchShaderIR* lhs,
                                 IZilchShaderIR* rhs,
                                 ZilchSpirVFrontEndContext* context);
  // Helpers to perform standard unary operations
  IZilchShaderIR* PerformUnaryOp(Zilch::UnaryOperatorNode*& node, OpType opType, ZilchSpirVFrontEndContext* context);
  IZilchShaderIR* PerformUnaryIncDecOp(Zilch::UnaryOperatorNode*& node,
                                       IZilchShaderIR* constantOne,
                                       OpType opType,
                                       ZilchSpirVFrontEndContext* context);
  // Helpers to perform standard typecast operations
  IZilchShaderIR* PerformTypeCast(Zilch::TypeCastNode*& node, OpType opType, ZilchSpirVFrontEndContext* context);

  ZilchShaderIROp* GetIntegerConstant(int value, ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* GetConstant(ZilchShaderIRType* type, StringParam value, ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* GetConstant(ZilchShaderIRType* type, Zilch::Any value, ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* ConstructCompositeFromScalar(BasicBlock* block,
                                                ZilchShaderIRType* compositeType,
                                                IZilchShaderIR* scalar,
                                                ZilchSpirVFrontEndContext* context);

  /// Create a specialization constant from a member variable node.
  ZilchShaderIROp* AddSpecializationConstant(Zilch::MemberVariableNode* node,
                                             ZilchShaderIRType* varType,
                                             ZilchSpirVFrontEndContext* context);
  /// Creates a specialization constant of the given type with the given unique
  /// lookup key. If a specialization constant is not a scalar, then it must be
  /// iteratively constructed from other specialization constants. If non-null,
  /// the given literal value will be used to initialize the constant (only
  /// valid for scalars).
  ZilchShaderIROp* AddSpecializationConstantRecursively(void* key,
                                                        ZilchShaderIRType* varType,
                                                        StringParam varName,
                                                        ZilchShaderIRConstantLiteral* literalValue,
                                                        Zilch::CodeLocation& codeLocation,
                                                        ZilchSpirVFrontEndContext* context);
  /// Create a specialization constant of a given variable type. The given key
  /// should be a unique identifier for this variable that can be used to fetch
  /// the specialization constant again at a later time. A null key specifies
  /// that there is no key (this variable can't be looked up later). This is
  /// typically used for sub-constants. The OpType should be specified to choose
  /// between scalar and composite constants.
  ZilchShaderIROp* CreateSpecializationConstant(void* key,
                                                OpType opType,
                                                ZilchShaderIRType* varType,
                                                ZilchSpirVFrontEndContext* context);

  void ToAny(ZilchShaderIRType* type, StringParam value, Zilch::Any& result);

  void ExtractDebugInfo(Zilch::SyntaxNode* node, ZilchShaderDebugInfo& debugInfo);
  void GetLineAsComment(Zilch::SyntaxNode* node, ZilchShaderIRComments& comments);
  BasicBlock* BuildBlock(StringParam labelDebugName, ZilchSpirVFrontEndContext* context);
  BasicBlock* BuildBlockNoStack(StringParam labelDebugName, ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildIROpNoBlockAdd(OpType opType,
                                       ZilchShaderIRType* resultType,
                                       ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp*
  BuildIROp(BasicBlock* block, OpType opType, ZilchShaderIRType* resultType, ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildIROp(BasicBlock* block,
                             OpType opType,
                             ZilchShaderIRType* resultType,
                             IZilchShaderIR* arg0,
                             ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildIROp(BasicBlock* block,
                             OpType opType,
                             ZilchShaderIRType* resultType,
                             IZilchShaderIR* arg0,
                             IZilchShaderIR* arg1,
                             ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildIROp(BasicBlock* block,
                             OpType opType,
                             ZilchShaderIRType* resultType,
                             IZilchShaderIR* arg0,
                             IZilchShaderIR* arg1,
                             IZilchShaderIR* arg2,
                             ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildCurrentBlockIROp(OpType opType,
                                         ZilchShaderIRType* resultType,
                                         ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildCurrentBlockIROp(OpType opType,
                                         ZilchShaderIRType* resultType,
                                         IZilchShaderIR* arg0,
                                         ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildCurrentBlockIROp(OpType opType,
                                         ZilchShaderIRType* resultType,
                                         IZilchShaderIR* arg0,
                                         IZilchShaderIR* arg1,
                                         ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildCurrentBlockIROp(OpType opType,
                                         ZilchShaderIRType* resultType,
                                         IZilchShaderIR* arg0,
                                         IZilchShaderIR* arg1,
                                         IZilchShaderIR* arg2,
                                         ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildCurrentBlockAccessChain(ZilchShaderIRType* baseResultType,
                                                ZilchShaderIROp* selfInstance,
                                                IZilchShaderIR* arg0,
                                                ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildCurrentBlockAccessChain(ZilchShaderIRType* baseResultType,
                                                ZilchShaderIROp* selfInstance,
                                                IZilchShaderIR* arg0,
                                                IZilchShaderIR* arg1,
                                                ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildDecorationOp(BasicBlock* block,
                                     IZilchShaderIR* decorationTarget,
                                     spv::Decoration decorationType,
                                     ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildDecorationOp(BasicBlock* block,
                                     IZilchShaderIR* decorationTarget,
                                     spv::Decoration decorationType,
                                     int decorationValue,
                                     ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildMemberDecorationOp(BasicBlock* block,
                                           IZilchShaderIR* decorationTarget,
                                           int memberOffset,
                                           spv::Decoration decorationType,
                                           ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildMemberDecorationOp(BasicBlock* block,
                                           IZilchShaderIR* decorationTarget,
                                           int memberOffset,
                                           spv::Decoration decorationType,
                                           int decorationValue,
                                           ZilchSpirVFrontEndContext* context);
  ZilchShaderIRConstantLiteral* GetOrCreateConstantIntegerLiteral(int value);
  ZilchShaderIRConstantLiteral* GetOrCreateConstantLiteral(Zilch::Any value);
  ZilchShaderIROp* BuildOpVariable(ZilchShaderIRType* resultType, ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* BuildOpVariable(BasicBlock* block,
                                   ZilchShaderIRType* resultType,
                                   int storageConstant,
                                   ZilchSpirVFrontEndContext* context);
  IZilchShaderIR* WalkAndGetResult(Zilch::SyntaxNode* node, ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* WalkAndGetValueTypeResult(BasicBlock* block,
                                             Zilch::SyntaxNode* node,
                                             ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* WalkAndGetValueTypeResult(Zilch::SyntaxNode* node, ZilchSpirVFrontEndContext* context);
  // If this is an immediate then the op is returned. If the op is a pointer
  // than a load is generated and the load is returned.
  ZilchShaderIROp* GetOrGenerateValueTypeFromIR(BasicBlock* block,
                                                IZilchShaderIR* instruction,
                                                ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* GetOrGenerateValueTypeFromIR(IZilchShaderIR* instruction, ZilchSpirVFrontEndContext* context);
  // If this is a pointer type (e.g. variable, parameter, etc...) then the op is
  // returned. Otherwise a new variable is generated, the immediate is stored
  // into it, and the variable is returned.
  ZilchShaderIROp* GetOrGeneratePointerTypeFromIR(BasicBlock* block,
                                                  IZilchShaderIR* instruction,
                                                  ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* GetOrGeneratePointerTypeFromIR(IZilchShaderIR* instruction, ZilchSpirVFrontEndContext* context);
  // Build an op to store the source into the target. May generate OpStore or
  // OpCopyMemor depending on the type of source.
  ZilchShaderIROp* BuildStoreOp(IZilchShaderIR* target,
                                IZilchShaderIR* source,
                                ZilchSpirVFrontEndContext* context,
                                bool forceLoadStore = true);
  ZilchShaderIROp* BuildStoreOp(BasicBlock* block,
                                IZilchShaderIR* target,
                                IZilchShaderIR* source,
                                ZilchSpirVFrontEndContext* context,
                                bool forceLoadStore = true);
  void GetFunctionCallArguments(Zilch::FunctionCallNode* node,
                                Zilch::MemberAccessNode* memberAccessNode,
                                Array<IZilchShaderIR*>& arguments,
                                ZilchSpirVFrontEndContext* context);
  void GetFunctionCallArguments(Zilch::FunctionCallNode* node,
                                IZilchShaderIR* thisOp,
                                Array<IZilchShaderIR*>& arguments,
                                ZilchSpirVFrontEndContext* context);
  void WriteFunctionCallArguments(Array<IZilchShaderIR*> arguments,
                                  ZilchShaderIRType* functionType,
                                  ZilchShaderIROp* functionCallOp,
                                  ZilchSpirVFrontEndContext* context);
  void WriteFunctionCallArgument(IZilchShaderIR* argument,
                                 ZilchShaderIROp* functionCallOp,
                                 ZilchShaderIRType* paramType,
                                 ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* GenerateFunctionCall(ZilchShaderIRFunction* shaderFunction, ZilchSpirVFrontEndContext* context);
  void WriteFunctionCallPostamble(ZilchSpirVFrontEndContext* context);
  void WriteFunctionCall(Array<IZilchShaderIR*> arguments,
                         ZilchShaderIRFunction* shaderFunction,
                         ZilchSpirVFrontEndContext* context);

  // Helpers
  ZilchShaderIROp* GenerateBoolToIntegerCast(BasicBlock* block,
                                             ZilchShaderIROp* source,
                                             ZilchShaderIRType* destType,
                                             ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* GenerateFromBoolCast(BasicBlock* block,
                                        ZilchShaderIROp* source,
                                        ZilchShaderIRType* destType,
                                        IZilchShaderIR* zero,
                                        IZilchShaderIR* one,
                                        ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* GenerateIntegerToBoolCast(BasicBlock* block,
                                             ZilchShaderIROp* source,
                                             ZilchShaderIRType* destType,
                                             ZilchSpirVFrontEndContext* context);
  ZilchShaderIROp* GenerateToBoolCast(BasicBlock* block,
                                      OpType op,
                                      ZilchShaderIROp* source,
                                      ZilchShaderIRType* destType,
                                      IZilchShaderIR* zero,
                                      ZilchSpirVFrontEndContext* context);

  // Get the value result type from an an op code. If this is a pointer type it
  // will get the dereference type.
  ZilchShaderIRType* GetResultValueType(ZilchShaderIROp* op);
  // Get the pointer result type from an an op code. If this is a value type it
  // will get the pointer type.
  ZilchShaderIRType* GetPointerValueType(ZilchShaderIROp* op);
  // Helper to check if the given type contains an attribute. If the type
  // doesn't have meta this returns false.
  bool ContainsAttribute(ZilchShaderIRType* shaderType, StringParam attributeName);
  // Check if a type contains the non-copyable attribute (returns true if
  // non-copyable). Throws an error if this type is non-copyable and optionally
  // generates a dummy variable.
  bool CheckForNonCopyableType(ZilchShaderIRType* shaderType,
                               Zilch::ExpressionNode* node,
                               ZilchSpirVFrontEndContext* context,
                               bool generateDummyResult = true);

  ZilchShaderIRType* FindType(Zilch::Type* type, Zilch::SyntaxNode* syntaxNode, bool reportErrors = true);
  ZilchShaderIRType* FindType(Zilch::ExpressionNode* syntaxNode, bool reportErrors = true);
  // Validate that the result type of the given instruction (must be an op) is
  // of a certain type (e.g. pointer).
  bool ValidateResultType(IZilchShaderIR* instruction,
                          ShaderIRTypeBaseType::Enum expectedType,
                          Zilch::CodeLocation& codeLocation,
                          bool throwException = true);
  /// Verifies that the given instruction can be written to. This includes
  /// checking for specialization constants.
  bool ValidateLValue(ZilchShaderIROp* op, Zilch::CodeLocation& codeLocation, bool throwException = true);

  // Generates a dummy instruction based upon the result type of the given node.
  // Used to make translation not crash after errors.
  IZilchShaderIR* GenerateDummyIR(Zilch::ExpressionNode* node, ZilchSpirVFrontEndContext* context);
  // Send a translation error with a simple message (also marks the translation
  // as having failed)
  void SendTranslationError(Zilch::CodeLocation& codeLocation, StringParam message);
  void SendTranslationError(Zilch::CodeLocation& codeLocation, StringParam shortMsg, StringParam fullMsg);
  // Send a translation error. If the location is null then a dummy location is
  // used (e.g. native types).
  void SendTranslationError(Zilch::CodeLocation* codeLocation, StringParam message);

  typedef Zilch::BranchWalker<ZilchSpirVFrontEnd, ZilchSpirVFrontEndContext> TranslatorBranchWalker;
  TranslatorBranchWalker mWalker;
  SimpleZilchParser mZilchCommentParser;
  ZilchSpirVFrontEndContext* mContext;

  ZilchShaderSpirVSettingsRef mSettings;
  ZilchShaderIRLibrary* mLibrary;
  ZilchShaderIRProject* mProject;

  // Was an error ever triggered?
  bool mErrorTriggered;
};

} // namespace Zero
