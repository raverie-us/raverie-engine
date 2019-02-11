// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class CycleDetection;

/// Context class for detecting cycles in the static call graph.
class CycleDetectionContext
    : public Zilch::WalkerContext<CycleDetection, CycleDetectionContext>
{
public:
  CycleDetectionContext();

  HashMap<Zilch::Type*, Zilch::ClassNode*> mTypeMap;
  HashMap<Zilch::Function*, Zilch::FunctionNode*> mFunctionMap;
  HashMap<Zilch::Function*, Zilch::ConstructorNode*> mConstructorMap;
  HashMap<Zilch::Property*, Zilch::MemberVariableNode*> mVariableMap;

  /// All objects that have been processed. Prevents visiting a part of
  /// the call graph multiple times (this can happen even if no cycles exist).
  HashSet<void*> mAllProcessedObjects;
  /// Keeps track of all objects in the current call stack (a duplicate means a
  /// cycle was found).
  HashSet<void*> mProcessedObjectsStack;
  /// Used to report the call stack that creates a cycle.
  Array<Zilch::SyntaxNode*> mCallStack;

  ShaderCompilationErrors* mErrors;
  ZilchShaderIRLibrary* mCurrentLibrary;
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
  CycleDetection(ZilchShaderSpirVSettings* settings);
  bool Run(Zilch::SyntaxTree& syntaxTree,
           ZilchShaderIRLibrary* library,
           ShaderCompilationErrors* errors);

protected:
  void PreWalkClassNode(Zilch::ClassNode*& node,
                        CycleDetectionContext* context);
  void PreWalkConstructor(Zilch::ConstructorNode*& node,
                          CycleDetectionContext* context);
  void PreWalkClassFunction(Zilch::FunctionNode*& node,
                            CycleDetectionContext* context);
  void PreWalkClassMemberVariable(Zilch::MemberVariableNode*& node,
                                  CycleDetectionContext* context);

  void WalkClassNode(Zilch::ClassNode*& node, CycleDetectionContext* context);
  void WalkClassPreconstructor(Zilch::ClassNode*& node,
                               CycleDetectionContext* context);
  void WalkClassPreconstructor(Zilch::Function* preConstructor,
                               CycleDetectionContext* context);
  void WalkClassConstructor(Zilch::ConstructorNode*& node,
                            CycleDetectionContext* context);
  void WalkClassFunction(Zilch::FunctionNode*& node,
                         CycleDetectionContext* context);
  void WalkClassMemberVariable(Zilch::MemberVariableNode*& node,
                               CycleDetectionContext* context);
  void WalkMemberAccessNode(Zilch::MemberAccessNode*& node,
                            CycleDetectionContext* context);
  void WalkStaticTypeNode(Zilch::StaticTypeNode*& node,
                          CycleDetectionContext* context);

  void ReportError(CycleDetectionContext* context);

  ShaderCompilationErrors* mErrors;
  ZilchShaderSpirVSettings* mSettings;
};

} // namespace Zero
