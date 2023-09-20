// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{
RaverieDefineStaticLibrary(Syntax)
{
  RaverieInitializeType(SyntaxNode);
  RaverieInitializeType(SyntaxType);
  RaverieInitializeType(AnySyntaxType);
  RaverieInitializeType(IndirectionSyntaxType);
  RaverieInitializeType(BoundSyntaxType);
  RaverieInitializeType(DelegateSyntaxType);
  RaverieInitializeType(RootNode);
  RaverieInitializeType(AttributeNode);
  RaverieInitializeType(StatementNode);
  RaverieInitializeType(ExpressionNode);
  RaverieInitializeType(BinaryOperatorNode);
  RaverieInitializeType(UnaryOperatorNode);
  RaverieInitializeType(PropertyDelegateOperatorNode);
  RaverieInitializeType(TypeCastNode);
  RaverieInitializeType(PostExpressionNode);
  RaverieInitializeType(IndexerCallNode);
  RaverieInitializeType(FunctionCallNode);
  RaverieInitializeType(MemberAccessNode);
  RaverieInitializeType(ExpressionInitializerNode);
  RaverieInitializeType(ExpressionInitializerMemberNode);
  RaverieInitializeType(ExpressionInitializerAddNode);
  RaverieInitializeType(MultiExpressionNode);
  RaverieInitializeType(VariableNode);
  RaverieInitializeType(LocalVariableNode);
  RaverieInitializeType(ParameterNode);
  RaverieInitializeType(MemberVariableNode);
  RaverieInitializeType(ValueNode);
  RaverieInitializeType(StringInterpolantNode);
  RaverieInitializeType(DeleteNode);
  RaverieInitializeType(ReturnNode);
  RaverieInitializeType(ScopeNode);
  RaverieInitializeType(StaticTypeNode);
  RaverieInitializeType(TimeoutNode);
  RaverieInitializeType(IfNode);
  RaverieInitializeType(IfRootNode);
  RaverieInitializeType(SendsEventNode);
  RaverieInitializeType(BreakNode);
  RaverieInitializeType(DebugBreakNode);
  RaverieInitializeType(ContinueNode);
  RaverieInitializeType(LoopScopeNode);
  RaverieInitializeType(ConditionalLoopNode);
  RaverieInitializeType(WhileNode);
  RaverieInitializeType(DoWhileNode);
  RaverieInitializeType(ForNode);
  RaverieInitializeType(ForEachNode);
  RaverieInitializeType(LoopNode);
  RaverieInitializeType(GenericFunctionNode);
  RaverieInitializeType(FunctionNode);
  RaverieInitializeType(InitializerNode);
  RaverieInitializeType(ConstructorNode);
  RaverieInitializeType(DestructorNode);
  RaverieInitializeType(ClassNode);
  RaverieInitializeType(TypeDefineNode);
  RaverieInitializeType(LocalVariableReferenceNode);
  RaverieInitializeType(ThrowNode);
  RaverieInitializeType(TypeIdNode);
  RaverieInitializeType(MemberIdNode);
  RaverieInitializeType(EnumValueNode);
  RaverieInitializeType(EnumNode);
}

RaverieDefineType(SyntaxType, builder, type)
{
}
RaverieDefineType(AnySyntaxType, builder, type)
{
}
RaverieDefineType(IndirectionSyntaxType, builder, type)
{
}
RaverieDefineType(BoundSyntaxType, builder, type)
{
}
RaverieDefineType(DelegateSyntaxType, builder, type)
{
}

RaverieDefineType(SyntaxNode, builder, type)
{
}
RaverieDefineType(RootNode, builder, type)
{
}
RaverieDefineType(AttributeNode, builder, type)
{
}
RaverieDefineType(StatementNode, builder, type)
{
}
RaverieDefineType(ExpressionNode, builder, type)
{
}
RaverieDefineType(BinaryOperatorNode, builder, type)
{
}
RaverieDefineType(UnaryOperatorNode, builder, type)
{
}
RaverieDefineType(PropertyDelegateOperatorNode, builder, type)
{
}
RaverieDefineType(TypeCastNode, builder, type)
{
}
RaverieDefineType(PostExpressionNode, builder, type)
{
}
RaverieDefineType(IndexerCallNode, builder, type)
{
}
RaverieDefineType(FunctionCallNode, builder, type)
{
}
RaverieDefineType(MemberAccessNode, builder, type)
{
}
RaverieDefineType(ExpressionInitializerNode, builder, type)
{
}
RaverieDefineType(ExpressionInitializerMemberNode, builder, type)
{
}
RaverieDefineType(ExpressionInitializerAddNode, builder, type)
{
}
RaverieDefineType(MultiExpressionNode, builder, type)
{
}
RaverieDefineType(VariableNode, builder, type)
{
}
RaverieDefineType(LocalVariableNode, builder, type)
{
}
RaverieDefineType(ParameterNode, builder, type)
{
}
RaverieDefineType(MemberVariableNode, builder, type)
{
}
RaverieDefineType(ValueNode, builder, type)
{
}
RaverieDefineType(StringInterpolantNode, builder, type)
{
}
RaverieDefineType(DeleteNode, builder, type)
{
}
RaverieDefineType(ReturnNode, builder, type)
{
}
RaverieDefineType(ScopeNode, builder, type)
{
}
RaverieDefineType(StaticTypeNode, builder, type)
{
}
RaverieDefineType(TimeoutNode, builder, type)
{
}
RaverieDefineType(IfNode, builder, type)
{
}
RaverieDefineType(IfRootNode, builder, type)
{
}
RaverieDefineType(SendsEventNode, builder, type)
{
}
RaverieDefineType(BreakNode, builder, type)
{
}
RaverieDefineType(DebugBreakNode, builder, type)
{
}
RaverieDefineType(ContinueNode, builder, type)
{
}
RaverieDefineType(LoopScopeNode, builder, type)
{
}
RaverieDefineType(ConditionalLoopNode, builder, type)
{
}
RaverieDefineType(WhileNode, builder, type)
{
}
RaverieDefineType(DoWhileNode, builder, type)
{
}
RaverieDefineType(ForNode, builder, type)
{
}
RaverieDefineType(ForEachNode, builder, type)
{
}
RaverieDefineType(LoopNode, builder, type)
{
}
RaverieDefineType(GenericFunctionNode, builder, type)
{
}
RaverieDefineType(FunctionNode, builder, type)
{
}
RaverieDefineType(InitializerNode, builder, type)
{
}
RaverieDefineType(ConstructorNode, builder, type)
{
}
RaverieDefineType(DestructorNode, builder, type)
{
}
RaverieDefineType(ClassNode, builder, type)
{
}
RaverieDefineType(TypeDefineNode, builder, type)
{
}
RaverieDefineType(LocalVariableReferenceNode, builder, type)
{
}
RaverieDefineType(ThrowNode, builder, type)
{
}
RaverieDefineType(TypeIdNode, builder, type)
{
}
RaverieDefineType(MemberIdNode, builder, type)
{
}
RaverieDefineType(EnumValueNode, builder, type)
{
}
RaverieDefineType(EnumNode, builder, type)
{
}

SyntaxTree::SyntaxTree() : SingleExpressionScope(nullptr), SingleExpressionIndex((size_t)-1)
{
  // Create the root node
  this->Root = new RootNode();
}

SyntaxTree::~SyntaxTree()
{
  delete this->Root;

  RaverieForEach (const UserToken* token, this->InvalidTokens)
    delete token;
}

SyntaxType::SyntaxType() : ResolvedType(nullptr)
{
}

bool SyntaxType::IsTemplateInstantiation() const
{
  // By default, we assume all syntax types are not templated
  return false;
}

String AnySyntaxType::ToString() const
{
  return Grammar::GetKeywordOrSymbol(Grammar::Any);
}

IndirectionSyntaxType::IndirectionSyntaxType() : ReferencedType(nullptr)
{
}

String IndirectionSyntaxType::ToString() const
{
  // Create a string builder since we're doing some concatenation
  StringBuilder output;

  // Add the 'ref' keyword
  output += Grammar::GetKeywordOrSymbol(Grammar::Ref);
  output += " ";

  // Add the type we're referencing
  output += this->ReferencedType->ToString();

  // Output the string
  return output.ToString();
}

void IndirectionSyntaxType::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->ReferencedType);
}

bool BoundSyntaxType::IsTemplateInstantiation() const
{
  return this->TemplateArguments.Size() > 0;
}

String BoundSyntaxType::ToString() const
{
  if (this->IsTemplateInstantiation())
  {
    StringBuilder builder;
    builder.Append(this->TypeName);

    builder.Append('[');

    for (size_t i = 0; i < this->TemplateArguments.Size(); ++i)
    {
      builder.Append(this->TemplateArguments[i]->ToString());

      if (i != this->TemplateArguments.Size() - 1)
      {
        builder.Append(", ");
      }
    }

    builder.Append(']');

    return builder.ToString();
  }
  else
  {
    return this->TypeName;
  }
}

void BoundSyntaxType::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->TemplateArguments.Populate(childrenOut);
}

DelegateSyntaxParameter::DelegateSyntaxParameter() : Name(nullptr), Type(nullptr)
{
}

DelegateSyntaxType::DelegateSyntaxType() : Return(nullptr)
{
}

bool DelegateSyntaxType::IsTemplateInstantiation() const
{
  return this->TemplateArguments.Size() > 0;
}

String DelegateSyntaxType::ToString() const
{
  // Create a string builder for concatenation
  StringBuilder output;

  // Output the delegate keyword and the opening argument parentheses
  output += Grammar::GetKeywordOrSymbol(Grammar::Delegate);
  output += " (";

  // Loop through all the parameters
  for (size_t i = 0; i < this->Parameters.Size(); ++i)
  {
    // Output the name of the parameter
    output += this->Parameters[i].Name->Token.c_str();
    output += " : ";

    // Always output the type of the parameter
    output += this->Parameters[i].Type->ToString();

    // If this is not the last parameter...
    if (i < this->Parameters.Size() - 1)
    {
      // Output an argument separator
      output += ", ";
    }
  }

  // Output the ending argument parentheses
  output += ")";

  // If we have a return type
  if (this->Return != nullptr)
  {
    // Output the return type...
    output += " : ";
    output += this->Return->ToString();
  }

  // Finally, output the full concatenated type string
  return output.ToString();
}

void DelegateSyntaxType::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Return);

  // Loop through all the parameters in the function
  for (size_t i = 0; i < this->Parameters.Size(); ++i)
  {
    childrenOut.Add(this->Parameters[i].Type);
  }
}

SyntaxNode::SyntaxNode() : Parent(nullptr), IsGenerated(false)
{
}

void SyntaxNode::DestroyChildren()
{
  // Get all the children for this node
  NodeChildren children;
  this->PopulateChildren(children);
  this->PopulateNonTraversedChildren(children);

  // Loop through all this node's children
  for (size_t i = 0; i < children.Size(); ++i)
  {
    // Get the current child
    SyntaxNode*& child = *children[i];

    // Destroy all of that child's children
    delete child;
    child = nullptr;
  }
}

SyntaxNode::SyntaxNode(const SyntaxNode& toCopy) : Parent(nullptr), Location(toCopy.Location)
{
  // We never copy the parent from other nodes
}

String SyntaxNode::ToString() const
{
  return RaverieVirtualTypeId(this)->Name;
}

void SyntaxNode::PopulateChildren(NodeChildren& /*childrenOut*/)
{
}

void SyntaxNode::PopulateNonTraversedChildren(NodeChildren& /*childrenOut*/)
{
}

void SyntaxNode::PopulateSyntaxTypes(SyntaxTypes& typesOut)
{
  NodeChildren children;
  this->PopulateChildren(children);
  this->PopulateNonTraversedChildren(children);

  // Walk through all the children we have
  for (size_t i = 0; i < children.Size(); ++i)
  {
    // Grab the current child and check if its a SyntaxType
    SyntaxNode** node = children[i];
    if (Type::DynamicCast<SyntaxType*>(*node) != nullptr)
    {
      // We know this is a syntax type, but we need a pointer to the pointer so
      // it can be possibly modified This is technically an unsafe cast, but we
      // 'soft' guarantee that we will only modify the second pointer with the
      // correct type
      const SyntaxType*& syntaxType = *(const SyntaxType**)node;
      typesOut.Add(syntaxType);
    }
  }
}

void SyntaxNode::FixParentPointers(SyntaxNode* node, SyntaxNode* parent)
{
  // Get all the children for this node
  NodeChildren children;
  node->PopulateChildren(children);

  // Loop through all this node's children
  for (size_t i = 0; i < children.Size(); ++i)
  {
    // Get the current child
    SyntaxNode* child = *children[i];

    // Destroy all of that child's children
    FixParentPointers(child, node);
  }

  // Setup the parent pointer for the current node
  node->Parent = parent;
}

String SyntaxNode::GetMergedComments()
{
  // Create a string builder
  StringBuilder builder;

  // Loop through all the comment strings
  for (size_t i = 0; i < this->Comments.Size(); ++i)
  {
    // Get the current comment
    String& comment = this->Comments[i];

    RaverieForEach (String line, comment.Split("\n"))
    {
      // Append the string range for the comment
      builder.Append(line.Trim());
      builder.Append("\n");
    }
  }

  // Return the merged comments
  return builder.ToString().Trim();
}

String ToNodeString(int nodeID)
{
  // Return the node name
  return String::Format("node%d", nodeID);
}

String NodeConnection(int nodeA, int nodeB)
{
  StringBuilder output;
  output += "  ";
  output += ToNodeString(nodeA);
  output += " -- ";
  output += ToNodeString(nodeB);
  output += "\n";
  return output.ToString();
}

String GetStart(int nodeID, const char* shape = "box")
{
  // Return the node starting text
  StringBuilder output;
  output += "  ";
  output += ToNodeString(nodeID);
  output += " [shape = ";
  output += shape;
  output += ", label = \"";
  return output.ToString();
}

String GetEnd()
{
  static const String ending = "\"];\n";
  return ending;
}

int GetGraphVizNodeRepresentation(StringBuilder& outString, int nodeID, SyntaxNode* node)
{
  outString += GetStart(nodeID);
  outString += node->ToString();
  outString += GetEnd();

  int childNodeID = nodeID + 1;

  NodeChildren children;
  node->PopulateChildren(children);

  for (size_t i = 0; i < children.Size(); ++i)
  {
    SyntaxNode* child = *children[i];

    outString += NodeConnection(nodeID, childNodeID);
    childNodeID = GetGraphVizNodeRepresentation(outString, childNodeID, child);
  }

  return childNodeID + 1;
}

void SyntaxTree::GetNodesAtCursor(size_t cursorPosition, StringParam cursorOrigin, Array<SyntaxNode*>& nodesOut)
{
  // Start at the root and walk down looking for nodes that our cursor is within
  nodesOut.Reserve(10);
  GetNodesAtCursorRecursive(this->Root, cursorPosition, cursorOrigin, nodesOut);
}

void SyntaxTree::GetNodesAtCursorRecursive(SyntaxNode* node,
                                           size_t cursorPosition,
                                           StringParam cursorOrigin,
                                           Array<SyntaxNode*>& nodesOut)
{
  // Check if the cursor is within the location of this node
  CodeLocation& location = node->Location;
  if (location.Origin == cursorOrigin && cursorPosition >= location.StartPosition &&
      cursorPosition <= location.EndPosition)
    nodesOut.PushBack(node);

  // Walk through all child nodes of the current node
  NodeChildren children;
  node->PopulateChildren(children);
  for (size_t i = 0; i < children.Size(); ++i)
  {
    // Recursively walk the tree looking for other nodes
    SyntaxNode* child = *children[i];
    GetNodesAtCursorRecursive(child, cursorPosition, cursorOrigin, nodesOut);
  }
}

String SyntaxTree::GetGraphVizRepresentation()
{
  StringBuilder output;
  output += "graph \"Syntax Tree\"\n{\n";

  GetGraphVizNodeRepresentation(output, 0, this->Root);

  output += "}";
  return output.ToString();
}

void RootNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Classes.Populate(childrenOut);
  this->Enums.Populate(childrenOut);
}

bool StatementNode::IsNodeUsedAsStatement(SyntaxNode* node)
{
  // If the node is an expression...
  ExpressionNode* expression = Type::DynamicCast<ExpressionNode*>(node);
  if (expression != nullptr)
  {
    // Return if the expression is being used as a statement
    return expression->IsUsedAsStatement;
  }

  // If the node is a function or class node... which, because it's a scope
  // node, is considered a statement (and should probably not be!)
  if (Type::DynamicCast<GenericFunctionNode*>(node) != nullptr || Type::DynamicCast<ClassNode*>(node) != nullptr)
  {
    return false;
  }

  // If the node is a member variable node... which, because it's a variable
  // node, is considered a statement (and should probably not be!)
  if (Type::DynamicCast<MemberVariableNode*>(node) != nullptr)
  {
    return false;
  }

  // Otherwise, just return if this node is a statement node
  return (Type::DynamicCast<StatementNode*>(node) != nullptr);
}

AttributeNode::AttributeNode() : TypeName(nullptr), AttributeCall(nullptr)
{
}

void AttributeNode::PopulateChildren(NodeChildren& childrenOut)
{
  childrenOut.Add(this->AttributeCall);
}

ExpressionNode::ExpressionNode() :
    ResultType(Core::GetInstance().ErrorType),
    PrecomputedResultType(nullptr),
    Io(IoMode::NotSet),
    IoUsage(IoMode::NotSet),
    IsUsedAsStatement(false)
{
}

BinaryOperatorNode::BinaryOperatorNode() : LeftOperand(nullptr), RightOperand(nullptr), Operator(nullptr)
{
}

String BinaryOperatorNode::ToString() const
{
  return this->Operator->Token;
}

void BinaryOperatorNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->LeftOperand);
  childrenOut.Add(this->RightOperand);
}

UnaryOperatorNode::UnaryOperatorNode() : Operand(nullptr), Operator(nullptr)
{
}

String UnaryOperatorNode::ToString() const
{
  return this->Operator->Token;
}

void UnaryOperatorNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Operand);
}

PropertyDelegateOperatorNode::PropertyDelegateOperatorNode() : AccessedProperty(nullptr)
{
}

TypeCastNode::TypeCastNode() : Operand(nullptr), Type(nullptr)
{
}

String TypeCastNode::ToString() const
{
  if (this->Type == nullptr)
    return "Implicit cast";
  return BuildString("Cast to ", this->Type->ToString());
}

void TypeCastNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Operand);
  childrenOut.Add(this->Type);
}

PostExpressionNode::PostExpressionNode() : LeftOperand(nullptr)
{
}

void PostExpressionNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->LeftOperand);
}

IndexerCallNode::IndexerCallNode() : Get(nullptr), GetSet(nullptr), Set(nullptr)
{
}

String IndexerCallNode::ToString() const
{
  return "[...]";
}

void IndexerCallNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Arguments.Populate(childrenOut);
  childrenOut.Add(this->Get);
  childrenOut.Add(this->GetSet);
  childrenOut.Add(this->Set);
}

FunctionCallNode::FunctionCallNode() : IsNamed(false)
{
}

String FunctionCallNode::ToString() const
{
  return "(...)";
}

void FunctionCallNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Arguments.Populate(childrenOut);
}

StaticTypeNode* FunctionCallNode::FindCreationCall()
{
  // If the left operand itself is the static type, then just return it directly
  if (StaticTypeNode* staticType = Type::DynamicCast<StaticTypeNode*>(this->LeftOperand))
    return staticType;

  // If this function call node is being used to invoke a constructor, we'll
  // know because our left operand will actually be a local variable whose
  // initial value is a CreationCallNode This is a bit complicated and maybe
  // should be refactored, but this works for now
  StaticTypeNode* creationNode = nullptr;
  LocalVariableNode* creationLocalVariable = Type::DynamicCast<LocalVariableNode*>(this->LeftOperand);

  // If the left operand was indeed a local variable, then look for a direct
  // reference to the CreationCallNode as the initial value
  if (creationLocalVariable != nullptr)
    return Type::DynamicCast<StaticTypeNode*>(creationLocalVariable->InitialValue);

  // We didn't find anything
  return nullptr;
}

MemberAccessNode::MemberAccessNode() :
    IsStatic(false),
    Operator(Grammar::Invalid),
    MemberType(MemberAccessType::Invalid),
    AccessedMember(nullptr),
    AccessedProperty(nullptr),
    AccessedGetterSetter(nullptr),
    AccessedField(nullptr),
    OverloadedFunctions(nullptr),
    AccessedFunction(nullptr)
{
}

String MemberAccessNode::ToString() const
{
  return BuildString(Grammar::GetKeywordOrSymbol(this->Operator), this->Name);
}

TypeIdNode::TypeIdNode() :
    CompileTimeSyntaxType(nullptr),
    Value(nullptr),
    CompileTimeType(Core::GetInstance().ErrorType)
{
}

void TypeIdNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Value);
  childrenOut.Add(this->CompileTimeSyntaxType);
}

MemberIdNode::MemberIdNode() : Member(nullptr)
{
}

void MemberIdNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Member);
}

StaticTypeNode::StaticTypeNode() :
    ReferencedSyntaxType(nullptr),
    ReferencedType(Core::GetInstance().ErrorType),
    Mode(CreationMode::Invalid),
    OverloadedConstructors(nullptr),
    ConstructorFunction(nullptr),
    ThisHandleLocal(0)
{
}

void StaticTypeNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->ReferencedSyntaxType);
}

ExpressionInitializerMemberNode::ExpressionInitializerMemberNode() : Value(nullptr)
{
}

void ExpressionInitializerMemberNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Value);
}

void ExpressionInitializerAddNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Arguments.Populate(childrenOut);
}

ExpressionInitializerNode::ExpressionInitializerNode()
{
}

void ExpressionInitializerNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->AddValues.Populate(childrenOut);
  this->InitailizeMembers.Populate(childrenOut);
  this->InitializerStatements.Populate(childrenOut);
}

MultiExpressionNode::MultiExpressionNode() : YieldChildExpressionIndex(InvalidIndex)
{
}

void MultiExpressionNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Expressions.Populate(childrenOut);
}

VariableNode::VariableNode() : InitialValue(nullptr), IsStatic(false), ResultSyntaxType(nullptr)
{
  // All variables can be both read from and written to
  this->Io = (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue);

  // Because we reuse this node as a statement in many explicit cases, then the
  // parent 'IoUsage' is always set to ignore Normally this should be set by
  // every parent, but variable nodes can only be used as expressions internally
  // (not in the parser)
  this->IoUsage = IoMode::Ignore;
}

bool VariableNode::IsInferred() const
{
  // We are inferred if we have no syntax type
  return this->ResultSyntaxType == nullptr;
}

LocalVariableNode::LocalVariableNode() : CreatedVariable(nullptr), ForwardLocalAccessIfPossible(false)
{
}

LocalVariableNode::LocalVariableNode(StringParam baseName,
                                     Project* parentProject,
                                     ExpressionNode* optionalInitialValue) :
    CreatedVariable(nullptr)
{
  this->IsGenerated = true;

  // The variable name includes the base name as well as brackets (so that the
  // user cannot type the name in via the parser)
  if (optionalInitialValue != nullptr)
    this->Name.Location = optionalInitialValue->Location;
  this->Name.Token =
      String::Format("[%s%llu]", baseName.c_str(), (unsigned long long)parentProject->VariableUniqueIdCounter);
  ++parentProject->VariableUniqueIdCounter;

  // This is entirely just to avoid a redudant copy into a local variable,
  // since the CreationCallNode already allocates stack space (and we only need
  // the local variable for the name lookup!)
  this->InitialValue = optionalInitialValue;
  this->ForwardLocalAccessIfPossible = true;
}

String VariableNode::ToString() const
{
  return this->Name.Token;
}

void VariableNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->InitialValue);
  childrenOut.Add(this->ResultSyntaxType);
}

ParameterNode::ParameterNode() : ParameterIndex(0)
{
}

MemberVariableNode::MemberVariableNode() :
    CreatedField(nullptr),
    CreatedGetterSetter(nullptr),
    CreatedProperty(nullptr),
    ParentClassType(nullptr),
    ResultType(Core::GetInstance().ErrorType),
    Get(nullptr),
    Set(nullptr),
    IsGetterSetter(false),
    ExtensionOwner(nullptr),
    Virtualized(VirtualMode::NonVirtual)
{
}

void MemberVariableNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Attributes.Populate(childrenOut);
  childrenOut.Add(this->Get);
  childrenOut.Add(this->Set);
  childrenOut.Add(this->ExtensionOwner);
}

ValueNode::ValueNode()
{
}

Type* ValueNode::PrecomputeType() const
{
  // Check to see what type of literal we have here
  switch (this->Value.TokenId)
  {
  case Grammar::IntegerLiteral:
    return RaverieTypeId(Integer);

  case Grammar::DoubleIntegerLiteral:
    return RaverieTypeId(DoubleInteger);

  case Grammar::RealLiteral:
    return RaverieTypeId(Real);

  case Grammar::DoubleRealLiteral:
    return RaverieTypeId(DoubleReal);

  case Grammar::StringLiteral:
    return RaverieTypeId(String);

  case Grammar::True:
  case Grammar::False:
    return RaverieTypeId(Boolean);

  case Grammar::Null:
    return RaverieTypeId(NullPointerType);
  default:
    break;
  }

  return nullptr;
}

String ValueNode::ToString() const
{
  return this->Value.Token;
}

StringInterpolantNode::StringInterpolantNode()
{
}

void StringInterpolantNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Elements.Populate(childrenOut);
}

DeleteNode::DeleteNode() : DeletedObject(nullptr)
{
}

void DeleteNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->DeletedObject);
}

ReturnNode::ReturnNode() : ReturnValue(nullptr), IsDebugReturn(false)
{
}

void ReturnNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->ReturnValue);
}

ScopeNode::ScopeNode() : AllPathsReturn(false), IsDebugReturn(false)
{
}

void ScopeNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Statements.Populate(childrenOut);
}

TimeoutNode::TimeoutNode() : Seconds(0)
{
}

IfNode::IfNode() : IsFirstPart(false), Condition(nullptr)
{
}

void IfNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Condition);
}

IfRootNode::IfRootNode()
{
}

void IfRootNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->IfParts.Populate(childrenOut);
}

SendsEventNode::SendsEventNode() : EventType(nullptr), Name(nullptr), EventProperty(nullptr)
{
}

void SendsEventNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->EventType);
  this->Attributes.Populate(childrenOut);
}

BreakNode::BreakNode() : ScopeCount(0), InstructionIndex(InvalidOpcodeLocation), JumpOpcode(nullptr)
{
}

ContinueNode::ContinueNode() : InstructionIndex(InvalidOpcodeLocation), JumpOpcode(nullptr)
{
}

ThrowNode::ThrowNode() : Exception(nullptr)
{
}

void ThrowNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Exception);
}

LoopScopeNode::LoopScopeNode()
{
}

LoopScopeNode::LoopScopeNode(const LoopScopeNode& toCopy) : ScopeNode(toCopy)
{
  // Error checking
  ErrorIf(toCopy.Breaks.Empty() == false, "You cannot copy a loop scope node with pointers to other break nodes");

  // Error checking
  ErrorIf(toCopy.Continues.Empty() == false,
          "You cannot copy a loop scope node with pointers to other continue "
          "nodes");
}

ConditionalLoopNode::ConditionalLoopNode() : Condition(nullptr)
{
}

void ConditionalLoopNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Condition);
}

ForNode::ForNode() : ValueVariable(nullptr), RangeVariable(nullptr), Initialization(nullptr), Iterator(nullptr)
{
}

void ForNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->ValueVariable);
  childrenOut.Add(this->RangeVariable);
  childrenOut.Add(this->Initialization);
  childrenOut.Add(this->Iterator);
}

ForEachNode::ForEachNode() : NonTraversedVariable(nullptr), NonTraversedRange(nullptr)
{
}

void ForEachNode::PopulateNonTraversedChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->NonTraversedVariable);
  childrenOut.Add(this->NonTraversedRange);
}

GenericFunctionNode::GenericFunctionNode() : Type(Core::GetInstance().ErrorDelegateType), DefinedFunction(nullptr)
{
}

void GenericFunctionNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Parameters.Populate(childrenOut);
  this->Attributes.Populate(childrenOut);
}

FunctionNode::FunctionNode() :
    ReturnType(nullptr),
    ExtensionOwner(nullptr),
    IsStatic(false),
    Virtualized(VirtualMode::NonVirtual)
{
}

String FunctionNode::ToString() const
{
  return BuildString("Function ", this->Name.Token);
}

void FunctionNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->ReturnType);
  childrenOut.Add(this->ExtensionOwner);
}

InitializerNode::InitializerNode() : InitializerType(nullptr), InitializerFunction(nullptr)
{
}

ConstructorNode::ConstructorNode() : BaseInitializer(nullptr), ThisInitializer(nullptr)
{
}

ClassNode::ClassNode() :
    CopyMode(TypeCopyMode::ReferenceType),
    Type(nullptr),
    Destructor(nullptr),
    PreConstructor(nullptr),
    TemplateInstantiation(nullptr)
{
}

bool ClassNode::IsTemplate() const
{
  return this->TemplateArguments.Size() > 0;
}

String ClassNode::ToString() const
{
  return BuildString("Class ", this->Name.Token);
}

void ClassNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Attributes.Populate(childrenOut);
  this->SendsEvents.Populate(childrenOut);
  this->Variables.Populate(childrenOut);
  this->Constructors.Populate(childrenOut);
  childrenOut.Add(this->Destructor);
  this->Functions.Populate(childrenOut);
  this->Inheritance.Populate(childrenOut);
}

EnumValueNode::EnumValueNode() : Value(nullptr), IntegralValue(0), IntegralProperty(nullptr)
{
}

String EnumValueNode::ToString() const
{
  // Return both the value name and the actual assigned integral value
  return String::Format("%s = %d", this->Name.c_str(), this->IntegralValue);
}

EnumNode::EnumNode() : IsFlags(false), Type(nullptr), Inheritance(nullptr)
{
}

String EnumNode::ToString() const
{
  return this->Name.Token;
}

void EnumNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  this->Values.Populate(childrenOut);
  this->Attributes.Populate(childrenOut);
  childrenOut.Add(this->Inheritance);
}

TypeDefineNode::TypeDefineNode() : Name(nullptr)
{
}

String TypeDefineNode::ToString() const
{
  return BuildString(this->Name->Token, " : ", this->Type->ToString());
}

void TypeDefineNode::PopulateChildren(NodeChildren& childrenOut)
{
  RaverieBase::PopulateChildren(childrenOut);
  childrenOut.Add(this->Type);
}

LocalVariableReferenceNode::LocalVariableReferenceNode() : AccessedVariable(nullptr)
{
}

String LocalVariableReferenceNode::ToString() const
{
  return BuildString("[Local Variable]", this->Value.Token);
}
} // namespace Raverie
