// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

CycleDetectionContext::CycleDetectionContext()
{
  mErrors = nullptr;
  mCurrentLibrary = nullptr;
}

CycleDetectionObjectScope::CycleDetectionObjectScope(
    void* object, CycleDetectionContext* context)
{
  mContext = context;
  mObject = object;
  PushScope();
}

CycleDetectionObjectScope::~CycleDetectionObjectScope()
{
  PopScope();
}

void CycleDetectionObjectScope::PushScope()
{
  mAlreadyVisited = false;
  mCycleDetected = false;

  // If we've visited this object twice in the DFS then we found a cycle.
  if (mContext->mProcessedObjectsStack.Contains(mObject))
  {
    mAlreadyVisited = true;
    mCycleDetected = true;
    return;
  }

  // Otherwise if we've already processed this object but not in the current
  // stack this means this entire sub-tree has already been processed and has no
  // cycles (new function calling into a fully explored call graph)
  if (mContext->mAllProcessedObjects.Contains(mObject))
  {
    mAlreadyVisited = true;
    return;
  }

  // Mark that we've visited this object
  mContext->mAllProcessedObjects.Insert(mObject);
  mContext->mProcessedObjectsStack.Insert(mObject);
}

void CycleDetectionObjectScope::PopScope()
{
  // This object is no longer part of the call stack so it's not an error if we
  // see it again
  mContext->mProcessedObjectsStack.Erase(mObject);
}

CycleDetection::CycleDetection(ZilchShaderSpirVSettings* settings)
{
  mErrors = nullptr;
  mSettings = settings;
}

bool CycleDetection::Run(Zilch::SyntaxTree& syntaxTree,
                         ZilchShaderIRLibrary* library,
                         ShaderCompilationErrors* errors)
{
  mErrors = errors;
  CycleDetectionContext context;
  context.mErrors = mErrors;
  context.mCurrentLibrary = library;

  // Do a pre-walk in order to build a map of nodes to zilch objects. This is
  // needed during that actual pass as we'll have to recurse from a function
  // call node into a function and all of its statements. Only a function node
  // has the actual function statements though so we have to build a map here.
  Zilch::BranchWalker<CycleDetection, CycleDetectionContext> preWalker;
  preWalker.Register(&CycleDetection::PreWalkClassNode);
  preWalker.Register(&CycleDetection::PreWalkConstructor);
  preWalker.Register(&CycleDetection::PreWalkClassFunction);
  preWalker.Register(&CycleDetection::PreWalkClassMemberVariable);
  preWalker.Walk(this, syntaxTree.Root, &context);

  Zilch::BranchWalker<CycleDetection, CycleDetectionContext> walker;
  walker.Register(&CycleDetection::WalkClassNode);
  walker.Register(&CycleDetection::WalkClassConstructor);
  walker.Register(&CycleDetection::WalkClassFunction);
  walker.Register(&CycleDetection::WalkClassMemberVariable);
  walker.Register(&CycleDetection::WalkMemberAccessNode);
  walker.Register(&CycleDetection::WalkStaticTypeNode);
  walker.Walk(this, syntaxTree.Root, &context);

  return mErrors->mErrorTriggered;
}

void CycleDetection::PreWalkClassNode(Zilch::ClassNode*& node,
                                      CycleDetectionContext* context)
{
  // Map the type to its node
  context->mTypeMap[node->Type] = node;
  // Walk all functions, constructors, and variables to do the same
  context->Walker->Walk(this, node->Functions, context);
  context->Walker->Walk(this, node->Constructors, context);
  context->Walker->Walk(this, node->Variables, context);
}

void CycleDetection::PreWalkConstructor(Zilch::ConstructorNode*& node,
                                        CycleDetectionContext* context)
{
  context->mConstructorMap[node->DefinedFunction] = node;
}

void CycleDetection::PreWalkClassFunction(Zilch::FunctionNode*& node,
                                          CycleDetectionContext* context)
{
  context->mFunctionMap[node->DefinedFunction] = node;
}

void CycleDetection::PreWalkClassMemberVariable(
    Zilch::MemberVariableNode*& node, CycleDetectionContext* context)
{
  context->mVariableMap[node->CreatedProperty] = node;
  // Additionally handle get/set functions.
  if (node->Get != nullptr)
    context->mFunctionMap[node->Get->DefinedFunction] = node->Get;
  if (node->Set != nullptr)
    context->mFunctionMap[node->Set->DefinedFunction] = node->Set;
}

void CycleDetection::WalkClassNode(Zilch::ClassNode*& node,
                                   CycleDetectionContext* context)
{
  WalkClassPreconstructor(node, context);
  context->Walker->Walk(this, node->Constructors, context);
  context->Walker->Walk(this, node->Functions, context);
}

void CycleDetection::WalkClassPreconstructor(Zilch::ClassNode*& node,
                                             CycleDetectionContext* context)
{
  Zilch::Function* preConstructor = node->PreConstructor;
  if (preConstructor == nullptr)
    return;

  CycleDetectionObjectScope objectScope(preConstructor, context);

  // If the node has been visited already then early out
  if (objectScope.mAlreadyVisited)
  {
    // If we're visiting this twice because of a cycle then report an error
    if (objectScope.mCycleDetected)
      ReportError(context);
    return;
  }

  // Continue the DFS by walking all member variables (which walks their
  // initializers)
  context->Walker->Walk(this, node->Variables, context);
}

void CycleDetection::WalkClassPreconstructor(Zilch::Function* preConstructor,
                                             CycleDetectionContext* context)
{
  // Pre-constructors require walking the class node.
  Zilch::ClassNode* classNode =
      context->mTypeMap.FindValue(preConstructor->Owner, nullptr);
  if (classNode != nullptr)
    WalkClassPreconstructor(classNode, context);
}

void CycleDetection::WalkClassConstructor(Zilch::ConstructorNode*& node,
                                          CycleDetectionContext* context)
{
  Zilch::Function* zilchConstructor = node->DefinedFunction;

  CycleDetectionObjectScope objectScope(zilchConstructor, context);
  // If the node has been visited already then early out
  if (objectScope.mAlreadyVisited)
  {
    // If we're visiting this twice because of a cycle then report an error
    if (objectScope.mCycleDetected)
      ReportError(context);
    return;
  }

  // Continue the DFS down the pre-construction function if it exists (instance
  // variable initializers)
  Zilch::Function* preConstructor = zilchConstructor->Owner->PreConstructor;
  if (preConstructor != nullptr)
  {
    // Push the constructor node onto the call stack as calling the
    // pre-constructor
    context->mCallStack.PushBack(node);
    WalkClassPreconstructor(preConstructor, context);
    context->mCallStack.PopBack();
  }
}

void CycleDetection::WalkClassFunction(Zilch::FunctionNode*& node,
                                       CycleDetectionContext* context)
{
  // Only walk a function once
  Zilch::Function* zilchFunction = node->DefinedFunction;

  CycleDetectionObjectScope objectScope(zilchFunction, context);
  // If the node has been visited already then early out
  if (objectScope.mAlreadyVisited)
  {
    // If we're visiting this twice because of a cycle then report an error
    if (objectScope.mCycleDetected)
      ReportError(context);
    return;
  }

  // Continue the DFS by walking all statements in the function
  context->Walker->Walk(this, node->Statements, context);
}

void CycleDetection::WalkClassMemberVariable(Zilch::MemberVariableNode*& node,
                                             CycleDetectionContext* context)
{
  // Visit the member variable. If the user requests to not continue iteration
  // then stop
  Zilch::Property* zilchProperty = node->CreatedProperty;

  CycleDetectionObjectScope objectScope(zilchProperty, context);
  // If the node has been visited already then early out
  if (objectScope.mAlreadyVisited)
  {
    // If we're visiting this twice because of a cycle then report an error
    if (objectScope.mCycleDetected)
      ReportError(context);
    return;
  }

  // Continue the DFS by walking variable initializers
  if (node->InitialValue != nullptr)
    context->Walker->Walk(this, node->InitialValue, context);
}

void CycleDetection::WalkMemberAccessNode(Zilch::MemberAccessNode*& node,
                                          CycleDetectionContext* context)
{
  // Find the actual accessed function/property
  Zilch::Function* zilchFunction = nullptr;
  Zilch::Property* zilchProperty = nullptr;
  // If this is actually a getter/setter, then find the called get/set function
  if (node->AccessedGetterSetter != nullptr)
  {
    if (node->IoUsage & Zilch::IoMode::ReadRValue)
      zilchFunction = node->AccessedGetterSetter->Get;
    else if (node->IoUsage & Zilch::IoMode::WriteLValue)
      zilchFunction = node->AccessedGetterSetter->Set;
  }
  else if (node->AccessedFunction != nullptr)
    zilchFunction = node->AccessedFunction;
  else if (node->AccessedProperty != nullptr)
    zilchProperty = node->AccessedProperty;

  // Always push the given node onto the current call stack
  context->mCallStack.PushBack(node);

  // If we're calling a function (including getter/setters)
  if (zilchFunction != nullptr)
  {
    // Deal with [Implements]. If an implements is registered we'll find a
    // shader function with a different zilch function then the one we started
    // with. This is the one that should actually be walked to find
    // dependencies.
    ZilchShaderIRFunction* shaderFunction =
        context->mCurrentLibrary->FindFunction(zilchFunction);
    if (shaderFunction != nullptr)
      zilchFunction = shaderFunction->mMeta->mZilchFunction;

    // Recursively walk the function we're calling if it's in the current
    // library. If it's not in the current library this will return null.
    Zilch::FunctionNode* fnNode =
        context->mFunctionMap.FindValue(zilchFunction, nullptr);
    if (fnNode != nullptr)
      context->Walker->Walk(this, fnNode, context);
  }
  // Otherwise, deal with member variables
  else if (zilchProperty != nullptr)
  {
    // Recursively walk the member we're referencing if it's in the current
    // library
    Zilch::MemberVariableNode* varNode =
        context->mVariableMap.FindValue(zilchProperty, nullptr);
    if (varNode != nullptr)
      context->Walker->Walk(this, varNode, context);
  }
  context->mCallStack.PopBack();
}

void CycleDetection::WalkStaticTypeNode(Zilch::StaticTypeNode*& node,
                                        CycleDetectionContext* context)
{
  // Always push the given node onto the current call stack
  context->mCallStack.PushBack(node);

  // If there's a constructor function then walk its dependencies
  Zilch::Function* constructor = node->ConstructorFunction;
  if (constructor != nullptr)
  {
    // Find the node for this constructor so we can walk the statements within.
    // If this node is null then the constructor call is from a different
    // library
    Zilch::ConstructorNode* constructorNode =
        context->mConstructorMap.FindValue(constructor, nullptr);
    if (constructorNode != nullptr)
      context->Walker->Walk(this, constructorNode, context);
  }
  // Otherwise this is a constructor call to a class with an implicit
  // constructor. In this case we have to traverse the pre-constructor.
  else
  {
    Zilch::Function* preConstructor = node->ReferencedType->PreConstructor;
    if (preConstructor != nullptr)
    {
      // Walk the pre-constructor to collect all its requirements
      WalkClassPreconstructor(preConstructor, context);
    }
  }
  context->mCallStack.PopBack();
}

void CycleDetection::ReportError(CycleDetectionContext* context)
{
  // Don't report duplicate errors
  if (context->mErrors->mErrorTriggered)
    return;

  ValidationErrorEvent toSend;
  toSend.mShortMessage = "Recursion is illegal in shaders";
  toSend.mFullMessage = "Object calls itself via the stack:";

  // Build the call stack from all of the call locations
  auto range = context->mCallStack.All();
  for (; !range.Empty(); range.PopFront())
  {
    Zilch::SyntaxNode* node = range.Front();
    toSend.mCallStack.PushBack(node->Location);
  }
  // Set the primary location to be the start of the recursion
  toSend.mLocation = toSend.mCallStack.Front();

  EventSend(context->mErrors, Events::ValidationError, &toSend);
  // Make sure to mark that we've triggered an error
  context->mErrors->mErrorTriggered = true;
}

} // namespace Zero
