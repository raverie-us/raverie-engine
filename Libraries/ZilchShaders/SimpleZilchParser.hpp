// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class SimpleZilchParser;

class SimpleZilchParserContext : public Zilch::WalkerContext<SimpleZilchParser, SimpleZilchParserContext>
{
public:
  StringBuilder mBuilder;
};

/// Simple parser for zilch that turns code into comments
class SimpleZilchParser
{
public:
  void Setup();

  String Run(Zilch::SyntaxNode* node);

  void WalkClassNode(Zilch::ClassNode*& node, SimpleZilchParserContext* context);
  void WalkClassVariables(Zilch::MemberVariableNode*& node, SimpleZilchParserContext* context);
  void WalkClassConstructor(Zilch::ConstructorNode*& node, SimpleZilchParserContext* context);
  void WalkClassFunction(Zilch::FunctionNode*& node, SimpleZilchParserContext* context);

  void WalkFunctionCallNode(Zilch::FunctionCallNode*& node, SimpleZilchParserContext* context);

  void WalkLocalVariable(Zilch::LocalVariableNode*& node, SimpleZilchParserContext* context);
  void WalkStaticTypeOrCreationCallNode(Zilch::StaticTypeNode*& node, SimpleZilchParserContext* context);
  void WalkExpressionInitializerNode(Zilch::ExpressionInitializerNode*& node, SimpleZilchParserContext* context);
  void WalkUnaryOperationNode(Zilch::UnaryOperatorNode*& node, SimpleZilchParserContext* context);
  void WalkBinaryOperationNode(Zilch::BinaryOperatorNode*& node, SimpleZilchParserContext* context);
  void WalkCastNode(Zilch::TypeCastNode*& node, SimpleZilchParserContext* context);
  void WalkValueNode(Zilch::ValueNode*& node, SimpleZilchParserContext* context);
  void WalkLocalRef(Zilch::LocalVariableReferenceNode*& node, SimpleZilchParserContext* context);
  void WalkGetterSetter(Zilch::MemberAccessNode*& node, Zilch::GetterSetter* getSet, SimpleZilchParserContext* context);
  void WalkMemberAccessNode(Zilch::MemberAccessNode*& node, SimpleZilchParserContext* context);
  void WalkMultiExpressionNode(Zilch::MultiExpressionNode*& node, SimpleZilchParserContext* context);

  void WalkIfRootNode(Zilch::IfRootNode*& node, SimpleZilchParserContext* context);
  void WalkIfNode(Zilch::IfNode*& node, SimpleZilchParserContext* context);
  void WalkBreakNode(Zilch::BreakNode*& node, SimpleZilchParserContext* context);
  void WalkContinueNode(Zilch::ContinueNode*& node, SimpleZilchParserContext* context);
  void WalkReturnNode(Zilch::ReturnNode*& node, SimpleZilchParserContext* context);
  void WalkWhileNode(Zilch::WhileNode*& node, SimpleZilchParserContext* context);
  void WalkDoWhileNode(Zilch::DoWhileNode*& node, SimpleZilchParserContext* context);
  void WalkForNode(Zilch::ForNode*& node, SimpleZilchParserContext* context);
  void WalkForEachNode(Zilch::ForEachNode*& node, SimpleZilchParserContext* context);

  typedef Zilch::BranchWalker<SimpleZilchParser, SimpleZilchParserContext> ParserBranchWalker;
  ParserBranchWalker mWalker;
};

} // namespace Zero
