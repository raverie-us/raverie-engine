///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------CodeRangeMapping
CodeRangeMapping::CodeRangeMapping()
{
  mSourcePositionStart = 0;
  mSourcePositionEnd = 0;
  mDestPositionStart = 0;
  mDestPositionEnd = 0;
  mIsRoot = false;
}

void CodeRangeMapping::Set(Zilch::CodeLocation& sourceLocation)
{
  mSourcePositionStart = sourceLocation.StartPosition;
  mSourcePositionEnd = sourceLocation.EndPosition;
  mSourceFile = sourceLocation.Origin;
}

//-------------------------------------------------------------------ScopedRangeMapping
ScopedRangeMapping::ScopedRangeMapping(ShaderCodeBuilder& builder, CodeRangeMapping* parent, Zilch::CodeLocation* zilchLocation, bool generateRanges, StringParam debugStr)
{
  mGenerateRanges = generateRanges;
  if(!mGenerateRanges)
    return;

  mBuilder = &builder;
  mTotalOffset = builder.GetSize();
  mParentTotalOffset = 0;
  mRange = &parent->mChildren.PushBack();

  mRange->mDestPositionStart = mTotalOffset;
  mRange->mDebugString = debugStr;
}

ScopedRangeMapping::ScopedRangeMapping(ShaderCodeBuilder& builder, ScopedRangeMapping* parent, CodeRangeMapping* rangeToCopy, Zilch::CodeLocation* zilchLocation, bool generateRanges, StringParam debugStr)
{
  mGenerateRanges = generateRanges;
  if(!mGenerateRanges)
    return;

  mBuilder = &builder;
  mParentTotalOffset = parent->mTotalOffset;
  mTotalOffset = builder.GetSize();

  mRange = &parent->mRange->mChildren.PushBack();
  if(rangeToCopy)
    *mRange = *rangeToCopy;

  mRange->mDestPositionStart = builder.GetSize() - parent->mTotalOffset;

  if(zilchLocation)
    mRange->Set(*zilchLocation);
  mRange->mDebugString = debugStr;
}

ScopedRangeMapping::~ScopedRangeMapping()
{
  if(!mGenerateRanges)
    return;

  mRange->mDestPositionEnd = mBuilder->GetSize() - mParentTotalOffset;
}

}//namespace Zero