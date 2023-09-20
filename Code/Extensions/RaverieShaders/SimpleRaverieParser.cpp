// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void SimpleRaverieParser::Setup()
{
  mWalker.Register(&SimpleRaverieParser::WalkClassNode);
  mWalker.Register(&SimpleRaverieParser::WalkClassVariables);
  mWalker.Register(&SimpleRaverieParser::WalkClassConstructor);
  mWalker.Register(&SimpleRaverieParser::WalkClassFunction);
  mWalker.Register(&SimpleRaverieParser::WalkFunctionCallNode);
  mWalker.Register(&SimpleRaverieParser::WalkLocalVariable);
  mWalker.Register(&SimpleRaverieParser::WalkStaticTypeOrCreationCallNode);
  mWalker.Register(&SimpleRaverieParser::WalkExpressionInitializerNode);
  mWalker.Register(&SimpleRaverieParser::WalkUnaryOperationNode);
  mWalker.Register(&SimpleRaverieParser::WalkBinaryOperationNode);
  mWalker.Register(&SimpleRaverieParser::WalkCastNode);
  mWalker.Register(&SimpleRaverieParser::WalkValueNode);
  mWalker.Register(&SimpleRaverieParser::WalkLocalRef);
  mWalker.Register(&SimpleRaverieParser::WalkMemberAccessNode);
  mWalker.Register(&SimpleRaverieParser::WalkIfRootNode);
  mWalker.Register(&SimpleRaverieParser::WalkIfNode);
  mWalker.Register(&SimpleRaverieParser::WalkContinueNode);
  mWalker.Register(&SimpleRaverieParser::WalkBreakNode);
  mWalker.Register(&SimpleRaverieParser::WalkReturnNode);
  mWalker.Register(&SimpleRaverieParser::WalkWhileNode);
  mWalker.Register(&SimpleRaverieParser::WalkDoWhileNode);
  mWalker.Register(&SimpleRaverieParser::WalkForNode);
  mWalker.Register(&SimpleRaverieParser::WalkForEachNode);
}

String SimpleRaverieParser::Run(Raverie::SyntaxNode* node)
{
  SimpleRaverieParserContext context;

  mWalker.Walk(this, node, &context);
  return context.mBuilder.ToString();
}

void SimpleRaverieParser::WalkClassNode(Raverie::ClassNode*& node, SimpleRaverieParserContext* context)
{
  if (node->CopyMode == Raverie::TypeCopyMode::ValueType)
    context->mBuilder << "struct " << node->Name.Token;
  else
    context->mBuilder << "class " << node->Name.Token;
}
void SimpleRaverieParser::WalkClassVariables(Raverie::MemberVariableNode*& node, SimpleRaverieParserContext* context)
{
}
void SimpleRaverieParser::WalkClassConstructor(Raverie::ConstructorNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << "constructor(";
  mWalker.Walk(this, node->Parameters, context);
  context->mBuilder << ")";
}
void SimpleRaverieParser::WalkClassFunction(Raverie::FunctionNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << "function " << node->Name.Token << "(";
  // Add all of the parameters
  size_t size = node->Parameters.Size();
  for (size_t i = 0; i < size; ++i)
  {
    Raverie::ParameterNode* paramNode = node->Parameters[i];
    context->mBuilder << paramNode->Name.Token << " : " << paramNode->ResultType->ToString();
    if (i != size - 1)
      context->mBuilder << ", ";
  }

  context->mBuilder << ")";
}
void SimpleRaverieParser::WalkFunctionCallNode(Raverie::FunctionCallNode*& node, SimpleRaverieParserContext* context)
{
  mWalker.Walk(this, node->LeftOperand, context);
  context->mBuilder << "(";
  mWalker.Walk(this, node->Arguments, context);
  context->mBuilder << ")";
}
void SimpleRaverieParser::WalkLocalVariable(Raverie::LocalVariableNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << "var " << node->Name.Token;
  if (node->InitialValue != nullptr)
  {
    context->mBuilder << " = ";
    mWalker.Walk(this, node->InitialValue, context);
  }
  context->mBuilder << ";";
}
void SimpleRaverieParser::WalkStaticTypeOrCreationCallNode(Raverie::StaticTypeNode*& node,
                                                         SimpleRaverieParserContext* context)
{
  context->mBuilder << node->ReferencedType->Name;
}
void SimpleRaverieParser::WalkExpressionInitializerNode(Raverie::ExpressionInitializerNode*& node,
                                                      SimpleRaverieParserContext* context)
{
}
void SimpleRaverieParser::WalkUnaryOperationNode(Raverie::UnaryOperatorNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << node->Operator->Token;
  context->mBuilder << "(";
  mWalker.Walk(this, node->Operand, context);
  context->mBuilder << ")";
}
void SimpleRaverieParser::WalkBinaryOperationNode(Raverie::BinaryOperatorNode*& node, SimpleRaverieParserContext* context)
{
  HashSet<int> noGroupingOps;
  noGroupingOps.Insert((int)Raverie::Grammar::Assignment);
  noGroupingOps.Insert((int)Raverie::Grammar::AssignmentAdd);
  noGroupingOps.Insert((int)Raverie::Grammar::AssignmentSubtract);
  noGroupingOps.Insert((int)Raverie::Grammar::AssignmentMultiply);
  noGroupingOps.Insert((int)Raverie::Grammar::AssignmentDivide);

  bool needsGrouping = !noGroupingOps.Contains(node->OperatorInfo.Operator);

  if (needsGrouping)
    context->mBuilder << "(";

  mWalker.Walk(this, node->LeftOperand, context);
  context->mBuilder << " " << node->Operator->Token << " ";
  mWalker.Walk(this, node->RightOperand, context);

  if (needsGrouping)
    context->mBuilder << ")";
}
void SimpleRaverieParser::WalkCastNode(Raverie::TypeCastNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << "(" << node->ResultType->ToString() << ")";
  mWalker.Walk(this, node->Operand, context);
}
void SimpleRaverieParser::WalkValueNode(Raverie::ValueNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << node->Value.Token;
}
void SimpleRaverieParser::WalkLocalRef(Raverie::LocalVariableReferenceNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << node->AccessedVariable->Name;
}
void SimpleRaverieParser::WalkGetterSetter(Raverie::MemberAccessNode*& node,
                                         Raverie::GetterSetter* getSet,
                                         SimpleRaverieParserContext* context)
{
}
void SimpleRaverieParser::WalkMemberAccessNode(Raverie::MemberAccessNode*& node, SimpleRaverieParserContext* context)
{
  mWalker.Walk(this, node->LeftOperand, context);
  context->mBuilder << Raverie::Grammar::GetKeywordOrSymbol(node->Operator);
  context->mBuilder << node->Name;
}
void SimpleRaverieParser::WalkMultiExpressionNode(Raverie::MultiExpressionNode*& node, SimpleRaverieParserContext* context)
{
}
void SimpleRaverieParser::WalkIfRootNode(Raverie::IfRootNode*& node, SimpleRaverieParserContext* context)
{
  // context->mBuilder << "if";
  // mWalker.Walk(this, node->IfParts, context);
}
void SimpleRaverieParser::WalkIfNode(Raverie::IfNode*& node, SimpleRaverieParserContext* context)
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
void SimpleRaverieParser::WalkBreakNode(Raverie::BreakNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << "break";
}
void SimpleRaverieParser::WalkContinueNode(Raverie::ContinueNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << "continue";
}
void SimpleRaverieParser::WalkReturnNode(Raverie::ReturnNode*& node, SimpleRaverieParserContext* context)
{
  context->mBuilder << "return";
  if (node->ReturnValue)
  {
    context->mBuilder << " ";
    mWalker.Walk(this, node->ReturnValue, context);
  }
}
void SimpleRaverieParser::WalkWhileNode(Raverie::WhileNode*& node, SimpleRaverieParserContext* context)
{
}
void SimpleRaverieParser::WalkDoWhileNode(Raverie::DoWhileNode*& node, SimpleRaverieParserContext* context)
{
}
void SimpleRaverieParser::WalkForNode(Raverie::ForNode*& node, SimpleRaverieParserContext* context)
{
}
void SimpleRaverieParser::WalkForEachNode(Raverie::ForEachNode*& node, SimpleRaverieParserContext* context)
{
}

} // namespace Raverie
