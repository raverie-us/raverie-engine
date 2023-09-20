// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class SimpleRaverieParser;

class SimpleRaverieParserContext : public Raverie::WalkerContext<SimpleRaverieParser, SimpleRaverieParserContext>
{
public:
  StringBuilder mBuilder;
};

/// Simple parser for raverie that turns code into comments
class SimpleRaverieParser
{
public:
  void Setup();

  String Run(Raverie::SyntaxNode* node);

  void WalkClassNode(Raverie::ClassNode*& node, SimpleRaverieParserContext* context);
  void WalkClassVariables(Raverie::MemberVariableNode*& node, SimpleRaverieParserContext* context);
  void WalkClassConstructor(Raverie::ConstructorNode*& node, SimpleRaverieParserContext* context);
  void WalkClassFunction(Raverie::FunctionNode*& node, SimpleRaverieParserContext* context);

  void WalkFunctionCallNode(Raverie::FunctionCallNode*& node, SimpleRaverieParserContext* context);

  void WalkLocalVariable(Raverie::LocalVariableNode*& node, SimpleRaverieParserContext* context);
  void WalkStaticTypeOrCreationCallNode(Raverie::StaticTypeNode*& node, SimpleRaverieParserContext* context);
  void WalkExpressionInitializerNode(Raverie::ExpressionInitializerNode*& node, SimpleRaverieParserContext* context);
  void WalkUnaryOperationNode(Raverie::UnaryOperatorNode*& node, SimpleRaverieParserContext* context);
  void WalkBinaryOperationNode(Raverie::BinaryOperatorNode*& node, SimpleRaverieParserContext* context);
  void WalkCastNode(Raverie::TypeCastNode*& node, SimpleRaverieParserContext* context);
  void WalkValueNode(Raverie::ValueNode*& node, SimpleRaverieParserContext* context);
  void WalkLocalRef(Raverie::LocalVariableReferenceNode*& node, SimpleRaverieParserContext* context);
  void WalkGetterSetter(Raverie::MemberAccessNode*& node, Raverie::GetterSetter* getSet, SimpleRaverieParserContext* context);
  void WalkMemberAccessNode(Raverie::MemberAccessNode*& node, SimpleRaverieParserContext* context);
  void WalkMultiExpressionNode(Raverie::MultiExpressionNode*& node, SimpleRaverieParserContext* context);

  void WalkIfRootNode(Raverie::IfRootNode*& node, SimpleRaverieParserContext* context);
  void WalkIfNode(Raverie::IfNode*& node, SimpleRaverieParserContext* context);
  void WalkBreakNode(Raverie::BreakNode*& node, SimpleRaverieParserContext* context);
  void WalkContinueNode(Raverie::ContinueNode*& node, SimpleRaverieParserContext* context);
  void WalkReturnNode(Raverie::ReturnNode*& node, SimpleRaverieParserContext* context);
  void WalkWhileNode(Raverie::WhileNode*& node, SimpleRaverieParserContext* context);
  void WalkDoWhileNode(Raverie::DoWhileNode*& node, SimpleRaverieParserContext* context);
  void WalkForNode(Raverie::ForNode*& node, SimpleRaverieParserContext* context);
  void WalkForEachNode(Raverie::ForEachNode*& node, SimpleRaverieParserContext* context);

  typedef Raverie::BranchWalker<SimpleRaverieParser, SimpleRaverieParserContext> ParserBranchWalker;
  ParserBranchWalker mWalker;
};

} // namespace Raverie
