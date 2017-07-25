///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#ifdef ZeroDebug
#define CodeRangeDebugString(format, ...) \
    String::Format(format, __VA_ARGS__)
#else
#define CodeRangeDebugString(format, ...) String()
#endif

namespace Zero
{

// Class used to track scopes to help preserve formatting across translation.
class ScopeLastNode
{
public:
  ScopeLastNode();

  Zilch::SyntaxNode* LastNode;
  Zilch::SyntaxNode* AssociatedScope;
};

class ZilchShaderTranslatorContext
  : public Zilch::WalkerContext<ZilchShaderTranslator, ZilchShaderTranslatorContext>
{
public:
  ZilchShaderTranslatorContext();
  ~ZilchShaderTranslatorContext();

  ShaderCodeBuilder& GetBuilder();

  /// There are a lot of random operations that need to traverse down just one portion of the tree
  /// and then collect that as a string. This is just a simple wrapper around creating a scoped
  /// builder just to get this information.
  String TranslateSubTree(ZilchShaderTranslator* translator, Zilch::SyntaxNode* node);

  void PushBuilder(ShaderCodeBuilder* builder);
  void PopBuilder();

  /// A stack of code builders so that various parts of the translator can collect
  /// portions of information by adding a builder for just it's operations.
  Array<ShaderCodeBuilder*> mCodeBuilders;
  ShaderCodeBuilder mFirstBuilder;

  ShaderType* mCurrentType;

  // At the top of this stack contains the last statement we processed
  // for the current scope we're in. Every time we enter a scope, a 
  // statement is pushed on the stack (first null, then the last statement)
  Array<ScopeLastNode> FormatScopes;

  // Sometimes auto-formatting needs to be turned off, such as during a for loop's declaration
  bool mPerformAutoFormatting;

  // The current set of ranges to append to when using the ScopedCodeRangeMapping. Typically a range's
  // destination values are considered to be relative to the parent in the stack, but occasionally a
  // range is added to start a new "parent" that isn't relative to its parent.
  Array<CodeRangeMapping*> mRangeMappingStack;
};

// Allows easy mapping of a sub-range.
class ScopedCodeRangeMapping
{
public:
  // This constructor creates a sub-range (based upon the last pushed code range) that maps
  // to the given zilch location. This should be the most commonly used constructor.
  ScopedCodeRangeMapping(ZilchShaderTranslatorContext* context, Zilch::CodeLocation& location, StringParam debugString = String());
  // Another common operation is to map a new destination range that maps to the same source location.
  ScopedCodeRangeMapping(ZilchShaderTranslatorContext* context, CodeRangeMapping& mapping);
  // The final common operation is to start a new rooted mapping. As the final source is pieced
  // together at the end, there's a lot of different ranges that have to be mapped at the end.
  // For instance, a function's final position will be mapped once in the final file when everything
  // is put together, but the signature also has to be used for a forward declaration.
  // Because of this a separate hierarchy is stored for the function signature (and another for the body)
  // that has its final destination position assembled when the final shader is made.
  ScopedCodeRangeMapping(ZilchShaderTranslatorContext* context, CodeRangeMapping* mapping, Zilch::CodeLocation& location, StringParam debugString = String());
  ~ScopedCodeRangeMapping();

  // Adds a new range as a child within the previous parent range. Used to add sub lines/expressions to any existing range.
  void PushChildRange(ZilchShaderTranslatorContext* context, int locationStart, int locationEnd, StringParam sourceFile, StringParam debugString = String());
  // Adds a new range that is not part of a child. Used to start mapping an independent string (such as a function signature).
  void PushExistingRange(ZilchShaderTranslatorContext* context, CodeRangeMapping* mapping, int locationStart, int locationEnd, StringParam sourceFile, StringParam debugString = String());

  // Early out the scoping of this range (in case we need to push another range afterwards)
  void PopRange();

  ZilchShaderTranslatorContext* mContext;
  // The offset within the code builder of where this range starts (relative to its parent)
  int mOffset;
};

}//namespace Zero
