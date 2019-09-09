// MIT Licensed (see LICENSE.md).

#pragma once

namespace Zero
{

class DebugGraphical : public Graphical
{
public:
  ZilchDeclareType(DebugGraphical, TypeCopyMode::ReferenceType);

  void Initialize(CogInitializer& initializer) override;

  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void AddToSpace() override;

  // Don't process component shader inputs
  void ComponentAdded(BoundType* typeId, Component* component) override
  {
  }
  void ComponentRemoved(BoundType* typeId, Component* component) override
  {
  }

  PrimitiveType::Enum mPrimitiveType;

  Array<Debug::DebugDrawObjectAny> mDebugObjects;
};

class DebugGraphicalPrimitive : public DebugGraphical
{
public:
  ZilchDeclareType(DebugGraphicalPrimitive, TypeCopyMode::ReferenceType);

  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
};

class DebugGraphicalThickLine : public DebugGraphicalPrimitive
{
public:
  ZilchDeclareType(DebugGraphicalThickLine, TypeCopyMode::ReferenceType);

  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
};

class DebugGraphicalText : public DebugGraphical
{
public:
  ZilchDeclareType(DebugGraphicalText, TypeCopyMode::ReferenceType);

  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
};

} // namespace Zero
