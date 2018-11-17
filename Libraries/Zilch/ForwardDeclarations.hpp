/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_FORWARD_DECLARATIONS_HPP
#define ZILCH_FORWARD_DECLARATIONS_HPP

namespace Zilch
{
  // Template declarations
  template <typename T>
  class ArrayClass;
  template <typename T>
  class HandleOf;
  template <typename Type, typename ModePolicy, typename DeletePolicy>
  class Ref;

  // Forward declarations
  class Any;
  class AnyType;
  class AttributeNode;
  class BinaryOperator;
  class BinaryOperatorNode;
  class BoundSyntaxType;
  class BoundType;
  class BreakNode;
  class Call;
  class CastOperator;
  class ClassContext;
  class ClassNode;
  class ClassType;
  class CodeFormat;
  class CodeFormatter;
  class CodeGenerator;
  class CodeLocation;
  class CollectorContext;
  class CompilationErrors;
  class Composition;
  class ConditionalLoopNode;
  class Constant;
  class ConstructorNode;
  class ContinueNode;
  class DebugBreakNode;
  class Debugger;
  class Delegate;
  class DelegateSyntaxParameter;
  class DelegateSyntaxType;
  class DelegateType;
  class DeleteNode;
  class DestructorNode;
  class DoWhileNode;
  class EnumNode;
  class EnumValueNode;
  class EventHandler;
  class Exception;
  class ExceptionEvent;
  class ExceptionReport;
  class ExecutableState;
  class ExpressionInitializerNode;
  class ExpressionNode;
  class FatalErrorEvent;
  class Field;
  class ForEachNode;
  class ForNode;
  class Function;
  class FunctionCallNode;
  class FunctionNode;
  class GeneratorContext;
  class GeneratorWalkerDatabase;
  class GenericFunctionNode;
  class GetterSetter;
  class Handle;
  class HandleManager;
  class HeapManager;
  class IdentifierNode;
  class IfNode;
  class IfRootNode;
  class IndexerCallNode;
  class IndirectionSyntaxType;
  class IndirectionType;
  class InitializerNode;
  class JsonBuilder;
  class JsonValue;
  class Library;
  class LibraryBuilder;
  class LocalVariableNode;
  class LocalVariableReferenceNode;
  class LoopNode;
  class LoopScopeNode;
  class Member;
  class MemberAccessNode;
  class MemberIdNode;
  class MemberVariableNode;
  class MemoryLeakEvent;
  class Module;
  class MultiExpressionNode;
  class Opcode;
  class OpcodeEvent;
  class Operand;
  class ParameterNode;
  class PerFrameData;
  class PerScopeData;
  class Plugin;
  class PointerManager;
  class PostExpressionNode;
  class Project;
  class Property;
  class PropertyDelegateOperatorNode;
  class ReflectionObject;
  class RelativeJumpOpcode;
  class Resolver;
  class ReturnNode;
  class RootNode;
  class ScopeNode;
  class ScriptingEnginePrivateData;
  class SendsEvent;
  class SendsEventNode;
  class StackData;
  class StackManager;
  class StatementNode;
  class StaticLibrary;
  class StaticTypeNode;
  class StringBuilderClass;
  class StringInterpolantNode;
  class StringManager;
  class Syntaxer;
  class SyntaxNode;
  class SyntaxTree;
  class SyntaxType;
  class ThrowNode;
  class TimeoutNode;
  class Tokenizer;
  class Type;
  class TypeBinding;
  class TypeCastNode;
  class TypeIdNode;
  class TypingContext;
  class UnaryOperator;
  class UnaryOperatorNode;
  class UnnamedOperandNode;
  class UserToken;
  class ValueNode;
  class Variable;
  class VariableNode;
  class WhileNode;
  class ZilchCodeBuilder;

  typedef const Module& ModuleParam;
}

#endif
