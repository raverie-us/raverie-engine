///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ScopeLastNode
ScopeLastNode::ScopeLastNode() :
  LastNode(nullptr),
  AssociatedScope(nullptr)
{
}

//-------------------------------------------------------------------ZilchShaderTranslator
ZilchShaderTranslatorContext::ZilchShaderTranslatorContext()
{
  mPerformAutoFormatting = true;
  PushBuilder(&mFirstBuilder);
}

ZilchShaderTranslatorContext::~ZilchShaderTranslatorContext()
{
  PopBuilder();
}

ShaderCodeBuilder& ZilchShaderTranslatorContext::GetBuilder()
{
  return *mCodeBuilders.Back();
}

String ZilchShaderTranslatorContext::TranslateSubTree(ZilchShaderTranslator* translator, Zilch::SyntaxNode* node)
{
  ScopedShaderCodeBuilder subBuilder(this);
  this->Walker->Walk(translator, node, this);
  subBuilder.PopFromStack();

  String result = subBuilder.ToString();
  return result;
}

void ZilchShaderTranslatorContext::PushBuilder(ShaderCodeBuilder* builder)
{
  mCodeBuilders.PushBack(builder);
}

void ZilchShaderTranslatorContext::PopBuilder()
{
  mCodeBuilders.PopBack();
}

//-------------------------------------------------------------------ScopedCodeRangeMapping
ScopedCodeRangeMapping::ScopedCodeRangeMapping(ZilchShaderTranslatorContext* context, Zilch::CodeLocation& location, StringParam debugString)
{
  PushChildRange(context, location.StartPosition, location.EndPosition, location.Origin, debugString);
}

ScopedCodeRangeMapping::ScopedCodeRangeMapping(ZilchShaderTranslatorContext* context, CodeRangeMapping& mapping)
{
  PushChildRange(context, mapping.mSourcePositionStart, mapping.mSourcePositionEnd, mapping.mSourceFile, mapping.mDebugString);
}

ScopedCodeRangeMapping::ScopedCodeRangeMapping(ZilchShaderTranslatorContext* context, CodeRangeMapping* mapping, Zilch::CodeLocation& location, StringParam debugString)
{
  PushExistingRange(context, mapping, location.StartPosition, location.EndPosition, location.Origin, debugString);
}

ScopedCodeRangeMapping::~ScopedCodeRangeMapping()
{
  PopRange();
}

void ScopedCodeRangeMapping::PushChildRange(ZilchShaderTranslatorContext* context, int locationStart, int locationEnd, StringParam sourceFile, StringParam debugString)
{
  mContext = context;

  ReturnIf(mContext->mRangeMappingStack.Empty(), , "Context ranges should never be empty");

  // Get the parent and create a new child. Also push that child as the current range on the stack.
  CodeRangeMapping* parent = mContext->mRangeMappingStack.Back();
  int parentIndex = mContext->mRangeMappingStack.Size() - 1;

  // Set the start position of the range. This is based upon how far we've written in the current builder,
  // but since we're a child of another range and our start should be relative to the parent then we need
  // to offset the destination start by the accumulated parent positions.
  mOffset = 0;
  // There could be multiple parents in the stack but we only want to walk until
  // we reach a "root". A root is any node that starts a new stack and ignores
  // the parents (such as a function signature within a class).
  while(parentIndex >= 0 && !mContext->mRangeMappingStack[parentIndex]->mIsRoot)
  {
    CodeRangeMapping* parent = mContext->mRangeMappingStack[parentIndex];
    mOffset += parent->mDestPositionStart;
    --parentIndex;
  }

  CodeRangeMapping* child = &parent->mChildren.PushBack();
  mContext->mRangeMappingStack.PushBack(child);

  child->mDestPositionStart = mContext->GetBuilder().GetSize() - mOffset;

  // Set all of the source location values
  child->mSourcePositionStart = locationStart;
  child->mSourcePositionEnd = locationEnd;
  child->mSourceFile = sourceFile;
  child->mDebugString = debugString;
  child->mIsRoot = false;
}

void ScopedCodeRangeMapping::PushExistingRange(ZilchShaderTranslatorContext* context, CodeRangeMapping* mapping, int locationStart, int locationEnd, StringParam sourceFile, StringParam debugString)
{
  mContext = context;

  mapping->mDestPositionStart = context->GetBuilder().GetSize();
  mapping->mSourcePositionStart = locationStart;
  mapping->mSourcePositionEnd = locationEnd;
  mapping->mSourceFile = sourceFile;
  mapping->mDebugString = debugString;
  mapping->mIsRoot = true;
  mOffset = 0;

  // Push the given range as the new "head" of the range stack
  mContext->mRangeMappingStack.PushBack(mapping);
}

void ScopedCodeRangeMapping::PopRange()
{
  // If this range was already popped (from a manual call instead of
  // the destructor for instance) then don't do anything.
  if(mContext == nullptr)
    return;

  // Safeguard against an empty stack
  ReturnIf(mContext->mRangeMappingStack.Empty(), , "Range stack should never be empty");

  // Pop the current range off the stack
  CodeRangeMapping* range = mContext->mRangeMappingStack.Back();
  mContext->mRangeMappingStack.PopBack();
  // Set the end position of the range now that we know how far we've written into the
  // builder but remember to offset it by the parent's start offset
  range->mDestPositionEnd = mContext->GetBuilder().GetSize() - mOffset;

  // Null out the context so we can't pop twice
  mContext = nullptr;
}

}//namespace Zero
