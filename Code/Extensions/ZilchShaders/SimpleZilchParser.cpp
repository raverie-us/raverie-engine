// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void SimpleZilchParser::Setup()
{
  mWalker.Register(&SimpleZilchParser::WalkClassNode);
  mWalker.Register(&SimpleZilchParser::WalkClassVariables);
  mWalker.Register(&SimpleZilchParser::WalkClassConstructor);
  mWalker.Register(&SimpleZilchParser::WalkClassFunction);
  mWalker.Register(&SimpleZilchParser::WalkFunctionCallNode);
  mWalker.Register(&SimpleZilchParser::WalkLocalVariable);
  mWalker.Register(&SimpleZilchParser::WalkStaticTypeOrCreationCallNode);
  mWalker.Register(&SimpleZilchParser::WalkExpressionInitializerNode);
  mWalker.Register(&SimpleZilchParser::WalkUnaryOperationNode);
  mWalker.Register(&SimpleZilchParser::WalkBinaryOperationNode);
  mWalker.Register(&SimpleZilchParser::WalkCastNode);
  mWalker.Register(&SimpleZilchParser::WalkValueNode);
  mWalker.Register(&SimpleZilchParser::WalkLocalRef);
  mWalker.Register(&SimpleZilchParser::WalkMemberAccessNode);
  mWalker.Register(&SimpleZilchParser::WalkIfRootNode);
  mWalker.Register(&SimpleZilchParser::WalkIfNode);
  mWalker.Register(&SimpleZilchParser::WalkContinueNode);
  mWalker.Register(&SimpleZilchParser::WalkBreakNode);
  mWalker.Register(&SimpleZilchParser::WalkReturnNode);
  mWalker.Register(&SimpleZilchParser::WalkWhileNode);
  mWalker.Register(&SimpleZilchParser::WalkDoWhileNode);
  mWalker.Register(&SimpleZilchParser::WalkForNode);
  mWalker.Register(&SimpleZilchParser::WalkForEachNode);
}

String SimpleZilchParser::Run(Zilch::SyntaxNode* node)
{
  SimpleZilchParserContext context;

  mWalker.Walk(this, node, &context);
  return context.mBuilder.ToString();
}

void SimpleZilchParser::WalkClassNode(Zilch::ClassNode*& node, SimpleZilchParserContext* context)
{
  if (node->CopyMode == Zilch::TypeCopyMode::ValueType)
    context->mBuilder << "struct " << node->Name.Token;
  else
    context->mBuilder << "class " << node->Name.Token;
}
void SimpleZilchParser::WalkClassVariables(Zilch::MemberVariableNode*& node, SimpleZilchParserContext* context)
{
}
void SimpleZilchParser::WalkClassConstructor(Zilch::ConstructorNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << "constructor(";
  mWalker.Walk(this, node->Parameters, context);
  context->mBuilder << ")";
}
void SimpleZilchParser::WalkClassFunction(Zilch::FunctionNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << "function " << node->Name.Token << "(";
  // Add all of the parameters
  size_t size = node->Parameters.Size();
  for (size_t i = 0; i < size; ++i)
  {
    Zilch::ParameterNode* paramNode = node->Parameters[i];
    context->mBuilder << paramNode->Name.Token << " : " << paramNode->ResultType->ToString();
    if (i != size - 1)
      context->mBuilder << ", ";
  }

  context->mBuilder << ")";
}
void SimpleZilchParser::WalkFunctionCallNode(Zilch::FunctionCallNode*& node, SimpleZilchParserContext* context)
{
  mWalker.Walk(this, node->LeftOperand, context);
  context->mBuilder << "(";
  mWalker.Walk(this, node->Arguments, context);
  context->mBuilder << ")";
}
void SimpleZilchParser::WalkLocalVariable(Zilch::LocalVariableNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << "var " << node->Name.Token;
  if (node->InitialValue != nullptr)
  {
    context->mBuilder << " = ";
    mWalker.Walk(this, node->InitialValue, context);
  }
  context->mBuilder << ";";
}
void SimpleZilchParser::WalkStaticTypeOrCreationCallNode(Zilch::StaticTypeNode*& node,
                                                         SimpleZilchParserContext* context)
{
  context->mBuilder << node->ReferencedType->Name;
}
void SimpleZilchParser::WalkExpressionInitializerNode(Zilch::ExpressionInitializerNode*& node,
                                                      SimpleZilchParserContext* context)
{
}
void SimpleZilchParser::WalkUnaryOperationNode(Zilch::UnaryOperatorNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << node->Operator->Token;
  context->mBuilder << "(";
  mWalker.Walk(this, node->Operand, context);
  context->mBuilder << ")";
}
void SimpleZilchParser::WalkBinaryOperationNode(Zilch::BinaryOperatorNode*& node, SimpleZilchParserContext* context)
{
  HashSet<int> noGroupingOps;
  noGroupingOps.Insert((int)Zilch::Grammar::Assignment);
  noGroupingOps.Insert((int)Zilch::Grammar::AssignmentAdd);
  noGroupingOps.Insert((int)Zilch::Grammar::AssignmentSubtract);
  noGroupingOps.Insert((int)Zilch::Grammar::AssignmentMultiply);
  noGroupingOps.Insert((int)Zilch::Grammar::AssignmentDivide);

  bool needsGrouping = !noGroupingOps.Contains(node->OperatorInfo.Operator);

  if (needsGrouping)
    context->mBuilder << "(";

  mWalker.Walk(this, node->LeftOperand, context);
  context->mBuilder << " " << node->Operator->Token << " ";
  mWalker.Walk(this, node->RightOperand, context);

  if (needsGrouping)
    context->mBuilder << ")";
}
void SimpleZilchParser::WalkCastNode(Zilch::TypeCastNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << "(" << node->ResultType->ToString() << ")";
  mWalker.Walk(this, node->Operand, context);
}
void SimpleZilchParser::WalkValueNode(Zilch::ValueNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << node->Value.Token;
}
void SimpleZilchParser::WalkLocalRef(Zilch::LocalVariableReferenceNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << node->AccessedVariable->Name;
}
void SimpleZilchParser::WalkGetterSetter(Zilch::MemberAccessNode*& node,
                                         Zilch::GetterSetter* getSet,
                                         SimpleZilchParserContext* context)
{
}
void SimpleZilchParser::WalkMemberAccessNode(Zilch::MemberAccessNode*& node, SimpleZilchParserContext* context)
{
  mWalker.Walk(this, node->LeftOperand, context);
  context->mBuilder << Zilch::Grammar::GetKeywordOrSymbol(node->Operator);
  context->mBuilder << node->Name;
}
void SimpleZilchParser::WalkMultiExpressionNode(Zilch::MultiExpressionNode*& node, SimpleZilchParserContext* context)
{
}
void SimpleZilchParser::WalkIfRootNode(Zilch::IfRootNode*& node, SimpleZilchParserContext* context)
{
  // context->mBuilder << "if";
  // mWalker.Walk(this, node->IfParts, context);
}
void SimpleZilchParser::WalkIfNode(Zilch::IfNode*& node, SimpleZilchParserContext* context)
{
  if (node->IsFirstPart)
    context->mBuilder << "if";
  else if (node->Condition == nullptr)
    context->mBuilder << "else";
  else
    context->mBuilder << "else if";

  if (node->Condition != nullptr)
  {
    context->mBuilder << "(";
    mWalker.Walk(this, node->Condition, context);
    context->mBuilder << ")";
  }
  // mWalker.Walk(this, node->IfParts, context);
}
void SimpleZilchParser::WalkBreakNode(Zilch::BreakNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << "break";
}
void SimpleZilchParser::WalkContinueNode(Zilch::ContinueNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << "continue";
}
void SimpleZilchParser::WalkReturnNode(Zilch::ReturnNode*& node, SimpleZilchParserContext* context)
{
  context->mBuilder << "return";
  if (node->ReturnValue)
  {
    context->mBuilder << " ";
    mWalker.Walk(this, node->ReturnValue, context);
  }
}
void SimpleZilchParser::WalkWhileNode(Zilch::WhileNode*& node, SimpleZilchParserContext* context)
{
}
void SimpleZilchParser::WalkDoWhileNode(Zilch::DoWhileNode*& node, SimpleZilchParserContext* context)
{
}
void SimpleZilchParser::WalkForNode(Zilch::ForNode*& node, SimpleZilchParserContext* context)
{
}
void SimpleZilchParser::WalkForEachNode(Zilch::ForEachNode*& node, SimpleZilchParserContext* context)
{
}

} // namespace Zero
