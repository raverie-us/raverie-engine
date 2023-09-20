// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class CycleDetection;

/// Context class for detecting cycles in the static call graph.
class CycleDetectionContext : public Raverie::WalkerContext<CycleDetection, CycleDetectionContext>
{
public:
  CycleDetectionContext();

  HashMap<Raverie::Type*, Raverie::ClassNode*> mTypeMap;
  HashMap<Raverie::Function*, Raverie::FunctionNode*> mFunctionMap;
  HashMap<Raverie::Function*, Raverie::ConstructorNode*> mConstructorMap;
  HashMap<Raverie::Property*, Raverie::MemberVariableNode*> mVariableMap;

  /// All objects that have been processed. Prevents visiting a part of
  /// the call graph multiple times (this can happen even if no cycles exist).
  HashSet<void*> mAllProcessedObjects;
  /// Keeps track of all objects in the current call stack (a duplicate means a
  /// cycle was found).
  HashSet<void*> mProcessedObjectsStack;
  /// Used to report the call stack that creates a cycle.
  Array<Raverie::SyntaxNode*> mCallStack;

  ShaderCompilationErrors* mErrors;
  RaverieShaderIRLibrary* mCurrentLibrary;
};

/// Helper class that manages pushing/popping an object from the current
/// call stack. Cleans up a lot of code in cycle detection.
class CycleDetectionObjectScope
{
public:
  CycleDetectionObjectScope(void* object, CycleDetectionContext* context);
  ~CycleDetectionObjectScope();

  void PushScope();
  void PopScope();

  CycleDetectionContext* mContext;
  void* mObject;
  /// Has the target object already been visited? Used to early
  /// out from the DFS. Set to true even if a cycle is detected.
  bool mAlreadyVisited;
  /// Did this object cause a cycle to be formed?
  bool mCycleDetected;
};

/// Helper class to detect cycles in the static call graph of a shader
/// library (which is illegal in shaders). Ideally this should be refactored to
/// two pieces: one that builds the static call graph which can be re-used for
/// multiple queries and then the actual cycle detection.
class CycleDetection
{
public:
  CycleDetection(RaverieShaderSpirVSettings* settings);
  bool Run(Raverie::SyntaxTree& syntaxTree, RaverieShaderIRLibrary* library, ShaderCompilationErrors* errors);

protected:
  void PreWalkClassNode(Raverie::ClassNode*& node, CycleDetectionContext* context);
  void PreWalkConstructor(Raverie::ConstructorNode*& node, CycleDetectionContext* context);
  void PreWalkClassFunction(Raverie::FunctionNode*& node, CycleDetectionContext* context);
  void PreWalkClassMemberVariable(Raverie::MemberVariableNode*& node, CycleDetectionContext* context);

  void WalkClassNode(Raverie::ClassNode*& node, CycleDetectionContext* context);
  void WalkClassPreconstructor(Raverie::ClassNode*& node, CycleDetectionContext* context);
  void WalkClassPreconstructor(Raverie::Function* preConstructor, CycleDetectionContext* context);
  void WalkClassConstructor(Raverie::ConstructorNode*& node, CycleDetectionContext* context);
  void WalkClassFunction(Raverie::FunctionNode*& node, CycleDetectionContext* context);
  void WalkClassMemberVariable(Raverie::MemberVariableNode*& node, CycleDetectionContext* context);
  void WalkMemberAccessNode(Raverie::MemberAccessNode*& node, CycleDetectionContext* context);
  void WalkStaticTypeNode(Raverie::StaticTypeNode*& node, CycleDetectionContext* context);

  void ReportError(CycleDetectionContext* context);

  ShaderCompilationErrors* mErrors;
  RaverieShaderSpirVSettings* mSettings;
};

} // namespace Raverie
