///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ShaderTypeData
// In the exceptional case where an error happens in the final translated shader instead of
// in zilch we need a way to map the shader's error location back to the source zilch script.
// To do this we keep a hierarchy of ranges mapping from the destination file to the source file.
// The hierarchy makes it easier to not map every line 1-1 but keep track of broader ranges.
struct CodeRangeMapping
{
  CodeRangeMapping();

  // Helper to copy source location from a zilch code location
  void Set(Zilch::CodeLocation& sourceLocation);

  // Where in the original zilch script this code came from
  int mSourcePositionStart;
  int mSourcePositionEnd;
  // Where in the final source (relative to our parent) this code maps to
  int mDestPositionStart;
  int mDestPositionEnd;
  // The original source file this came from
  String mSourceFile;
  // This root variable is only needed when building up code ranges so we know when to append parent offsets.
  bool mIsRoot;

  // When looking at the hierarchy of ranges it can be really hard to determine what a range
  // was for. This is a debug string to help me keep track of why a range was constructed.
  String mDebugString;

  // To make it easy to map all sorts of ranges they are stored in a hierarchy. Ever child is
  // defined as a sub-range within its parent. This also allows easy mapping of all code locations.
  // If there is a position that is within this node but not within all the children then the
  // line maps to the full range of the parent.
  // @JoshD: An array by value isn't stable as some scoped ranges want to grab pointers. Use a mem pool?
  Array<CodeRangeMapping> mChildren;
};

//-------------------------------------------------------------------ScopedRangeMapping
// A helper class to build range mappings while appending to a ShaderCodeBuilder. This class is scoped to
// auto compute start/end positions. Currently the CodeRangeMapping is not stable if you push more than
// one range in the same scope (as it's an array) so you cannot push more than one ScopedRangeMapping on in a given scope. @JoshD
class ScopedRangeMapping
{
public:
  ScopedRangeMapping(ShaderCodeBuilder& builder, CodeRangeMapping* parent, Zilch::CodeLocation* zilchLocation, bool generateRanges, StringParam debugStr = String());
  ScopedRangeMapping(ShaderCodeBuilder& builder, ScopedRangeMapping* parent, CodeRangeMapping* rangeToCopy, Zilch::CodeLocation* zilchLocation, bool generateRanges, StringParam debugStr = String());
  ~ScopedRangeMapping();

  // The current code builder that is being appended to
  ShaderCodeBuilder* mBuilder;
  // The range we are keeping track of with the current scope
  CodeRangeMapping* mRange;
  // Keep track of our total offset from the current builder (so a child knows how to compute its offset)
  int mTotalOffset;
  // Keep track of the total offset that our parent was within the builder. As our current range is
  // defined under our parent we need to know how to offset the builder's current size to get a parent relative position.
  int mParentTotalOffset;
  // Optimizations: Should we generate ranges at all?
  bool mGenerateRanges;
};

//-------------------------------------------------------------------ShaderTypeTranslation
// Simple struct used to represent a translated shader type. The code range mapping will be
// filled out depending on the flags passed to a translator.
class ShaderTypeTranslation
{
public:
  // The final translation string of the shader.
  String mTranslation;
  // The range mapping of character positions in the shader back to the source zilch scripts (where possible).
  CodeRangeMapping mRoot;
};

}//namespace Zero
